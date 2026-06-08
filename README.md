# AI 車輛與行人辨識警示系統 C++ 版

這是程式設計期末報告使用的 C++ 專案。

本專案想做的是一個「車輛與行人接近時的警示系統」。目前版本先用 CSV 模擬影像辨識後得到的人車位置，程式會讀取每一個時間點的行人與車輛框，再計算距離、速度、接近方向與危險區域，最後輸出危險等級。

這樣做的原因是：期末報告的重點是把 C++ 程式邏輯寫清楚，並且能穩定展示判斷流程。`cars.xml` 保留在專案中，代表原本車輛辨識模型的素材，之後可以再接到 OpenCV 影像辨識流程。

## 專案檔案

| 檔案 | 說明 |
|---|---|
| `src/main.cpp` | C++ 主程式，負責讀取資料、計算危險分數、輸出結果 |
| `data/sample_detections.csv` | 範例偵測資料 |
| `CMakeLists.txt` | CMake 編譯設定 |
| `build_windows.bat` | Windows 編譯腳本 |
| `build_mac.sh` | Mac 編譯腳本 |
| `cars.xml` | 車輛辨識模型素材，保留作為未來影像辨識延伸 |
| `reports/` | 期末報告 Word 與簡報檔 |

## 執行環境

需要：

- C++17 編譯器
- Windows：Visual Studio Build Tools 或 Visual Studio Community 的 C++ 工具
- Mac：系統的 `c++` 編譯器，或另外安裝 CMake

本程式目前不需要安裝 OpenCV，因為這版展示是先使用 CSV 模擬辨識結果。

## Windows 編譯與執行

在 PowerShell 或命令提示字元進入專案資料夾：

```powershell
cd "C:\Users\webbt\Documents\程式期末"
```

編譯：

```powershell
.\build_windows.bat
```

使用內建 demo 資料執行：

```powershell
.\build\traffic_safety_ai.exe --demo
```

使用 CSV 範例資料執行：

```powershell
.\build\traffic_safety_ai.exe --csv data\sample_detections.csv
```

## Mac 編譯與執行

進入專案資料夾：

```zsh
cd ~/Desktop/程式期末
```

第一次執行前，先讓腳本可以執行：

```zsh
chmod +x build_mac.sh
```

編譯：

```zsh
./build_mac.sh
```

使用內建 demo 資料執行：

```zsh
./build-mac/traffic_safety_ai --demo
```

使用 CSV 範例資料執行：

```zsh
./build-mac/traffic_safety_ai --csv data/sample_detections.csv
```

如果想把像素距離換算成公尺與公里時，可以加上比例參數。這份範例 CSV 把每個 `frame` 當成 1 秒一個時間點，所以這裡也明確設定 `--fps 1`：

```zsh
./build-mac/traffic_safety_ai --csv data/sample_detections.csv --meters-per-pixel 0.05 --fps 1
```

`--fps` 代表每秒有幾個 frame，可依照資料來源調整，程式最高允許設定到 50 fps：

```zsh
./build-mac/traffic_safety_ai --csv data/sample_detections.csv --meters-per-pixel 0.05 --fps 50
```

## CSV 資料格式

CSV 的每一列代表某一個時間點偵測到的一個物件。

| 欄位 | 說明 |
|---|---|
| `frame` | 第幾個時間點 |
| `type` | 物件種類，目前使用 `person` 或 `vehicle` |
| `id` | 物件編號，例如 `p1`、`c1` |
| `x` | 偵測框左上角 x 座標 |
| `y` | 偵測框左上角 y 座標 |
| `w` | 偵測框寬度 |
| `h` | 偵測框高度 |
| `confidence` | 偵測信心值，範圍約 0 到 1 |

範例資料中，行人 `p1` 站在固定位置，車輛 `c1` 從左邊逐漸靠近行人，所以危險等級會慢慢升高。

## 程式判斷流程

程式主要流程如下：

1. 讀取 demo 或 CSV 資料。
2. 依照 `frame` 把同一個時間點的人與車找出來。
3. 把每個偵測框轉成中心點。
4. 計算行人中心點與車輛中心點的距離。
5. 用上一個 `frame` 的車輛位置估算車輛移動速度。
6. 判斷車輛是否正在接近行人。
7. 在行人周圍建立危險區域，判斷車輛是否進入。
8. 將距離、速度、接近方向、危險區域與信心值加成危險分數。
9. 依照分數輸出 `SAFE`、`WATCH`、`DANGER` 或 `CRITICAL`。

## 危險分數怎麼算

程式不是只看一個條件，而是把多個條件加起來。

| 條件 | 加分概念 |
|---|---|
| 人車距離很近 | 距離越近，分數越高 |
| 車輛進入危險區域 | 車輛框碰到行人周圍的警戒範圍時加分 |
| 車輛正在接近行人 | 這一個 frame 的距離比上一個 frame 更近時加分 |
| 車輛速度較快 | 移動越快，危險分數越高 |
| 偵測信心值 | 信心值越高，代表資料越可信，所以會給少量加分 |

分數最高會限制在 100 分，不會超過 100。

危險等級：

| 分數 | 等級 | 意思 |
|---|---|---|
| 0 - 34 | `SAFE` | 安全 |
| 35 - 64 | `WATCH` | 需要注意 |
| 65 - 89 | `DANGER` | 危險 |
| 90 - 100 | `CRITICAL` | 非常危險 |

## 輸出結果怎麼看

程式執行後會輸出每一個時間點最危險的一組人車結果。

| 欄位 | 說明 |
|---|---|
| `frame` | 第幾個時間點 |
| `level` | 危險等級 |
| `score` | 危險分數 |
| `pair` | 哪個行人與哪台車，例如 `p1/c1` |
| `distance` | 人車距離 |
| `speed` | 車輛估算速度 |
| `alert` | 對應警示燈與聲音 |
| `reason` | 這次判斷危險的主要原因 |

## 期末報告重點

本次期末專案主要完成：

- 將原本的車輛與行人警示概念整理成 C++ 程式。
- 使用 CSV 讓資料輸入穩定，方便課堂展示。
- 設計人車距離、速度、接近方向與危險區域的判斷方法。
- 將危險程度轉成清楚的等級與警示輸出。
- 保留 `cars.xml`，作為未來接入影像辨識的延伸素材。
