# AI 車輛與行人辨識警示系統

本專案是程式設計期末專題的 C++ 實作。程式會讀取車輛與行人的偵測資料，根據人車距離、車輛是否靠近、是否進入行人周圍危險區域，以及車輛移動速度，計算每個時間點的風險等級。

目前版本使用 CSV 檔模擬影像辨識後取得的偵測框資料，重點放在 C++ 判斷邏輯與輸出結果。`cars.xml` 保留作為之後接 OpenCV 車輛辨識時可延伸使用的模型素材。

## 專案檔案

| 路徑 | 說明 |
|---|---|
| `src/main.cpp` | 主程式 |
| `data/sample_detections.csv` | 範例偵測資料 |
| `CMakeLists.txt` | CMake 編譯設定 |
| `build_mac.sh` | macOS 編譯腳本 |
| `build_windows.bat` | Windows 編譯腳本 |
| `cars.xml` | 車輛辨識模型素材 |

## 資料格式

CSV 每一列代表某個時間點偵測到的一個物件。

| 欄位 | 說明 |
|---|---|
| `frame` | 時間點編號 |
| `type` | 物件類型，使用 `person` 或 `vehicle` |
| `id` | 物件編號 |
| `x` | 偵測框左上角 x 座標 |
| `y` | 偵測框左上角 y 座標 |
| `w` | 偵測框寬度 |
| `h` | 偵測框高度 |
| `confidence` | 偵測信心值 |

範例資料中，行人 `p1` 固定在同一位置，車輛 `c1` 從左側逐漸靠近行人。

## 判斷方式

程式主要依照以下條件計算風險分數：

1. 人車中心點距離。
2. 車輛是否進入行人周圍的危險區域。
3. 車輛是否比上一個時間點更靠近行人。
4. 車輛移動速度。
5. 偵測信心值。

風險分數會轉成四種等級：

| 分數 | 等級 | 警示 |
|---|---|---|
| 0 - 34 | `SAFE` | 無警示 |
| 35 - 64 | `WATCH` | 黃燈提醒 |
| 65 - 89 | `DANGER` | 紅燈與提示音 |
| 90 - 100 | `CRITICAL` | 紅燈閃爍與快速提示音 |

## 編譯

macOS：

```zsh
chmod +x build_mac.sh
./build_mac.sh
```

Windows：

```powershell
.\build_windows.bat
```

## 執行

macOS：

```zsh
./build-mac/traffic_safety_ai --csv data/sample_detections.csv --meters-per-pixel 0.05 --fps 1
```

Windows：

```powershell
.\build\traffic_safety_ai.exe --csv data\sample_detections.csv --meters-per-pixel 0.05 --fps 1
```

參數說明：

| 參數 | 說明 |
|---|---|
| `--csv <path>` | 指定 CSV 偵測資料 |
| `--meters-per-pixel <value>` | 像素轉公尺比例 |
| `--fps <value>` | 每秒 frame 數，最高限制為 50 |

這份範例資料把每個 `frame` 視為 1 秒一個時間點，所以展示時使用 `--fps 1`。

## 輸出內容

程式會輸出每個時間點風險最高的一組人車配對。

| 欄位 | 說明 |
|---|---|
| `frame` | 時間點 |
| `level` | 風險等級 |
| `score` | 風險分數 |
| `pair` | 行人與車輛配對 |
| `distance` | 人車距離 |
| `speed` | 車輛估算速度 |
| `alert` | 警示輸出 |
| `reason` | 判斷原因 |

## 目前限制

目前尚未直接串接攝影機或即時影像辨識，CSV 資料是用來模擬辨識後的人車位置。後續可以把 OpenCV 偵測結果接到相同資料格式，再使用目前的風險判斷邏輯輸出警示。
