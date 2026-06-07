#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Box {
    double x = 0;
    double y = 0;
    double w = 0;
    double h = 0;
};

struct Point {
    double x = 0;
    double y = 0;
};

struct Detection {
    int frame = 0;
    string type;   // person or vehicle
    string id;     // p1, c1...
    Box box;
    double confidence = 1.0;
};

struct RiskResult {
    int frame = 0;
    string level = "SAFE";
    double score = 0;
    string pair = "-/-";
    double distancePixels = 0;
    double speedPixelsPerSecond = 0;
    string reason = "no person-vehicle pair";
};

Point getCenter(Box box) {
    Point center;
    center.x = box.x + box.w / 2.0;
    center.y = box.y + box.h / 2.0;
    return center;
}

double getDistance(Point a, Point b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

bool boxesOverlap(Box a, Box b) {
    double left = max(a.x, b.x);
    double right = min(a.x + a.w, b.x + b.w);
    double top = max(a.y, b.y);
    double bottom = min(a.y + a.h, b.y + b.h);
    return right > left && bottom > top;
}

Box makeHazardZone(Box personBox) {
    Box zone;
    zone.w = personBox.w * 3.0;
    zone.h = personBox.h * 1.6;
    zone.x = personBox.x - (zone.w - personBox.w) / 2.0;
    zone.y = personBox.y - (zone.h - personBox.h) / 2.0;
    return zone;
}

string riskLevel(double score) {
    if (score >= 90) {
        return "CRITICAL";
    }
    if (score >= 65) {
        return "DANGER";
    }
    if (score >= 35) {
        return "WATCH";
    }
    return "SAFE";
}

string alertText(string level) {
    if (level == "CRITICAL") {
        return "LED:FLASHING_RED SOUND:FAST_BEEP";
    }
    if (level == "DANGER") {
        return "LED:RED SOUND:BEEP";
    }
    if (level == "WATCH") {
        return "LED:YELLOW SOUND:OFF";
    }
    return "LED:OFF SOUND:OFF";
}

vector<string> split(string line, char delimiter) {
    vector<string> parts;
    string part;
    stringstream ss(line);

    while (getline(ss, part, delimiter)) {
        parts.push_back(part);
    }

    return parts;
}

vector<Detection> loadCsv(string filename) {
    vector<Detection> detections;
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Cannot open CSV file: " << filename << endl;
        return detections;
    }

    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        vector<string> data = split(line, ',');
        if (data.size() != 8) {
            continue;
        }

        Detection d;
        d.frame = stoi(data[0]);
        d.type = data[1];
        d.id = data[2];
        d.box.x = stod(data[3]);
        d.box.y = stod(data[4]);
        d.box.w = stod(data[5]);
        d.box.h = stod(data[6]);
        d.confidence = stod(data[7]);
        detections.push_back(d);
    }

    return detections;
}

vector<Detection> demoData() {
    vector<Detection> data;

    data.push_back({0, "person", "p1", {290, 180, 50, 120}, 0.94});
    data.push_back({0, "vehicle", "c1", {40, 190, 120, 70}, 0.90});

    data.push_back({1, "person", "p1", {290, 180, 50, 120}, 0.95});
    data.push_back({1, "vehicle", "c1", {95, 190, 120, 70}, 0.92});

    data.push_back({2, "person", "p1", {290, 180, 50, 120}, 0.96});
    data.push_back({2, "vehicle", "c1", {160, 188, 120, 72}, 0.93});

    data.push_back({3, "person", "p1", {290, 180, 50, 120}, 0.96});
    data.push_back({3, "vehicle", "c1", {225, 186, 120, 74}, 0.94});

    data.push_back({4, "person", "p1", {290, 180, 50, 120}, 0.96});
    data.push_back({4, "vehicle", "c1", {280, 184, 120, 74}, 0.94});

    data.push_back({5, "person", "p1", {290, 180, 50, 120}, 0.96});
    data.push_back({5, "vehicle", "c1", {340, 184, 120, 74}, 0.94});

    return data;
}

vector<Detection> getFrameObjects(vector<Detection> detections, int frame, string type) {
    vector<Detection> result;

    for (Detection d : detections) {
        if (d.frame == frame && d.type == type) {
            result.push_back(d);
        }
    }

    return result;
}

bool findPreviousVehicle(vector<Detection> detections, Detection vehicle, Detection& previous) {
    for (Detection d : detections) {
        if (d.type == "vehicle" && d.id == vehicle.id && d.frame == vehicle.frame - 1) {
            previous = d;
            return true;
        }
    }

    return false;
}

RiskResult evaluateRisk(
    Detection person,
    Detection vehicle,
    vector<Detection> allDetections,
    double fps,
    double metersPerPixel
) {
    RiskResult result;
    result.frame = person.frame;
    result.pair = person.id + "/" + vehicle.id;

    Point personCenter = getCenter(person.box);
    Point vehicleCenter = getCenter(vehicle.box);
    result.distancePixels = getDistance(personCenter, vehicleCenter);

    double score = 0;
    vector<string> reasons;

    if (metersPerPixel > 0) {
        double meters = result.distancePixels * metersPerPixel;
        if (meters < 3) {
            score += 45;
            reasons.push_back("distance < 3m");
        } else if (meters < 6) {
            score += 30;
            reasons.push_back("distance < 6m");
        } else if (meters < 10) {
            score += 15;
            reasons.push_back("distance < 10m");
        }
    } else {
        if (result.distancePixels < 90) {
            score += 45;
            reasons.push_back("very close");
        } else if (result.distancePixels < 170) {
            score += 28;
            reasons.push_back("close");
        } else if (result.distancePixels < 260) {
            score += 12;
            reasons.push_back("nearby");
        }
    }

    Box hazardZone = makeHazardZone(person.box);
    if (boxesOverlap(hazardZone, vehicle.box)) {
        score += 25;
        reasons.push_back("vehicle in hazard zone");
    }

    Detection previousVehicle;
    if (findPreviousVehicle(allDetections, vehicle, previousVehicle)) {
        Point previousCenter = getCenter(previousVehicle.box);
        double movedPixels = getDistance(previousCenter, vehicleCenter);
        result.speedPixelsPerSecond = movedPixels * fps;

        double previousDistance = getDistance(previousCenter, personCenter);
        bool approaching = result.distancePixels + 3 < previousDistance;

        if (approaching) {
            score += 20;
            reasons.push_back("vehicle is approaching");
        }

        if (metersPerPixel > 0) {
            double kmh = result.speedPixelsPerSecond * metersPerPixel * 3.6;
            if (kmh > 30) {
                score += 25;
                reasons.push_back("speed > 30km/h");
            } else if (kmh > 15) {
                score += 12;
                reasons.push_back("speed > 15km/h");
            }
        } else {
            if (result.speedPixelsPerSecond > 550) {
                score += 20;
                reasons.push_back("fast movement");
            } else if (result.speedPixelsPerSecond > 250) {
                score += 10;
                reasons.push_back("moving");
            }
        }
    }

    score += ((person.confidence + vehicle.confidence) / 2.0) * 10.0;
    score = min(score, 100.0);

    result.score = score;
    result.level = riskLevel(score);

    if (!reasons.empty()) {
        result.reason = reasons[0];
        for (int i = 1; i < static_cast<int>(reasons.size()); i++) {
            result.reason += "; " + reasons[i];
        }
    } else {
        result.reason = "low risk";
    }

    return result;
}

vector<RiskResult> analyze(vector<Detection> detections, double fps, double metersPerPixel) {
    vector<RiskResult> results;
    int maxFrame = 0;

    for (Detection d : detections) {
        if (d.frame > maxFrame) {
            maxFrame = d.frame;
        }
    }

    for (int frame = 0; frame <= maxFrame; frame++) {
        vector<Detection> people = getFrameObjects(detections, frame, "person");
        vector<Detection> vehicles = getFrameObjects(detections, frame, "vehicle");

        RiskResult best;
        best.frame = frame;

        for (Detection person : people) {
            for (Detection vehicle : vehicles) {
                RiskResult current = evaluateRisk(person, vehicle, detections, fps, metersPerPixel);
                if (current.score > best.score) {
                    best = current;
                }
            }
        }

        results.push_back(best);
    }

    return results;
}

void printResults(vector<RiskResult> results, double metersPerPixel) {
    cout << "frame  level     score  pair        distance    speed       alert" << endl;
    cout << "-----  --------  -----  ----------  ----------  ----------  -----------------------------" << endl;

    for (RiskResult r : results) {
        cout << setw(5) << r.frame << "  ";
        cout << left << setw(8) << r.level << right << "  ";
        cout << setw(5) << fixed << setprecision(1) << r.score << "  ";
        cout << left << setw(10) << r.pair << right << "  ";

        if (metersPerPixel > 0) {
            cout << left << setw(10) << (to_string(r.distancePixels * metersPerPixel).substr(0, 5) + " m") << right << "  ";
            cout << left << setw(10) << (to_string(r.speedPixelsPerSecond * metersPerPixel * 3.6).substr(0, 5) + " km/h") << right << "  ";
        } else {
            cout << left << setw(10) << (to_string(r.distancePixels).substr(0, 5) + " px") << right << "  ";
            cout << left << setw(10) << (to_string(r.speedPixelsPerSecond).substr(0, 5) + " px/s") << right << "  ";
        }

        cout << alertText(r.level) << endl;
        cout << "       reason: " << r.reason << endl;
    }
}

void printHelp() {
    cout << "AI vehicle and pedestrian warning system" << endl;
    cout << endl;
    cout << "Usage:" << endl;
    cout << "  ./traffic_safety_ai --demo" << endl;
    cout << "  ./traffic_safety_ai --csv data/sample_detections.csv" << endl;
    cout << "  ./traffic_safety_ai --csv data/sample_detections.csv --meters-per-pixel 0.05" << endl;
}

int main(int argc, char* argv[]) {
    vector<Detection> detections;
    double fps = 10.0;
    double metersPerPixel = 0.0;

    if (argc == 1) {
        detections = demoData();
    }

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "--demo") {
            detections = demoData();
        } else if (arg == "--csv" && i + 1 < argc) {
            detections = loadCsv(argv[i + 1]);
            i++;
        } else if (arg == "--meters-per-pixel" && i + 1 < argc) {
            metersPerPixel = stod(argv[i + 1]);
            i++;
        } else if (arg == "--fps" && i + 1 < argc) {
            fps = stod(argv[i + 1]);
            i++;
        }
    }

    if (detections.empty()) {
        cout << "No detection data. Please use --demo or --csv." << endl;
        printHelp();
        return 1;
    }

    vector<RiskResult> results = analyze(detections, fps, metersPerPixel);
    printResults(results, metersPerPixel);

    return 0;
}
