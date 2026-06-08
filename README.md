# AI 車輛與行人辨識警示系統

本專案為程式設計期末專題的 C++ 實作。目標是模擬一套車輛與行人接近時的警示系統：程式讀取每個時間點偵測到的人與車位置，計算人車距離、車輛速度、接近方向與危險區域，最後輸出風險分數與警示等級。

目前版本使用 CSV 檔模擬影像辨識後的偵測結果，讓展示時可以穩定驗證 C++ 判斷邏輯。`cars.xml` 保留作為後續接入 OpenCV 車輛辨識流程的模型素材。

## 專案內容

| 路徑 | 說明 |
|---|---|
| `src/main.cpp` | C++ 主程式，包含資料讀取、風險計算與結果輸出 |
| `data/sample_detections.csv` | 範例偵測資料，模擬行人與車輛在不同時間點的位置 |
| `CMakeLists.txt` | CMake 編譯設定 |
| `build_mac.sh` | macOS / Unix-like 環境編譯腳本 |
| `build_windows.bat` | Windows 編譯腳本 |
| `cars.xml` | 車輛辨識模型素材，供未來 OpenCV 延伸使用 |

本 repository 只保留程式碼、範例資料與建置設定，不包含簡報或報告文件。

## 系統流程

1. 讀取內建 demo 資料或 CSV 偵測資料。
2. 依照 `frame` 將同一時間點的行人與車輛分組。
3. 將每個偵測框轉換成中心點。
4. 計算行人中心點與車輛中心點的距離。
5. 根據上一個 frame 的車輛位置估算移動速度。
6. 判斷車輛是否正在接近行人。
7. 在行人周圍建立危險區域，檢查車輛是否進入警戒範圍。
8. 根據距離、速度、接近狀態、危險區域與偵測信心值計算風險分數。
9. 將風險分數轉換為 `SAFE`、`WATCH`、`DANGER` 或 `CRITICAL`。

## 風險判斷邏輯

程式不是只用單一條件判斷危險，而是將多個因素加權成 0 到 100 的風險分數。

| 判斷因素 | 說明 |
|---|---|
| 人車距離 | 距離越近，分數越高 |
| 危險區域 | 車輛框進入行人周圍警戒範圍時提高分數 |
| 接近方向 | 若車輛與行人的距離比前一個 frame 更短，視為正在接近 |
| 車輛速度 | 移動速度越高，分數越高 |
| 偵測信心值 | 偵測結果信心值越高，會加入少量加權 |

風險等級對應如下：

| 分數範圍 | 等級 | 警示輸出 |
|---|---|---|
| 0 - 34 | `SAFE` | `LED:OFF SOUND:OFF` |
| 35 - 64 | `WATCH` | `LED:YELLOW SOUND:OFF` |
| 65 - 89 | `DANGER` | `LED:RED SOUND:BEEP` |
| 90 - 100 | `CRITICAL` | `LED:FLASHING_RED SOUND:FAST_BEEP` |

## CSV 資料格式

`data/sample_detections.csv` 的每一列代表某個時間點偵測到的一個物件。

| 欄位 | 說明 |
|---|---|
| `frame` | 時間點編號 |
| `type` | 物件種類，目前支援 `person` 與 `vehicle` |
| `id` | 物件編號，例如 `p1`、`c1` |
| `x` | 偵測框左上角 x 座標 |
| `y` | 偵測框左上角 y 座標 |
| `w` | 偵測框寬度 |
| `h` | 偵測框高度 |
| `confidence` | 偵測信心值，範圍約為 0 到 1 |

範例資料設計為行人 `p1` 站在固定位置，車輛 `c1` 從左側逐漸靠近行人，因此輸出結果會呈現風險等級逐步升高的過程。

## 編譯方式

### macOS / Unix-like

```zsh
chmod +x build_mac.sh
./build_mac.sh
```

若系統有 CMake，腳本會使用 CMake 建置；若沒有 CMake，會改用系統 C++ 編譯器直接編譯。

### Windows

```powershell
.\build_windows.bat
```

Windows 環境需安裝 Visual Studio Build Tools 或 Visual Studio Community 的 C++ 工具。

## 執行方式

使用內建 demo 資料：

```zsh
./build-mac/traffic_safety_ai --demo
```

使用 CSV 範例資料：

```zsh
./build-mac/traffic_safety_ai --csv data/sample_detections.csv
```

將像素距離換算成公尺與公里時速：

```zsh
./build-mac/traffic_safety_ai --csv data/sample_detections.csv --meters-per-pixel 0.05 --fps 1
```

參數說明：

| 參數 | 說明 |
|---|---|
| `--demo` | 使用程式內建測試資料 |
| `--csv <path>` | 使用指定 CSV 偵測資料 |
| `--meters-per-pixel <value>` | 將像素距離換算成公尺的比例 |
| `--fps <value>` | 每秒 frame 數，最高限制為 50 |

這份範例 CSV 將每個 `frame` 視為 1 秒一個時間點，因此展示公尺與時速時建議使用 `--fps 1`。程式仍支援最高 50 fps，以便未來接入真實影像串流或不同資料來源。

## 輸出欄位

程式執行後會輸出每個 frame 中風險最高的一組人車配對。

| 欄位 | 說明 |
|---|---|
| `frame` | 時間點編號 |
| `level` | 風險等級 |
| `score` | 風險分數 |
| `pair` | 行人與車輛配對，例如 `p1/c1` |
| `distance` | 人車距離 |
| `speed` | 車輛估算速度 |
| `alert` | 對應警示燈與聲音 |
| `reason` | 判斷為該風險等級的主要原因 |

## 目前完成項目

- 使用 C++17 實作人車接近警示邏輯。
- 支援內建 demo 與 CSV 輸入。
- 支援像素距離與實際距離換算。
- 根據距離、速度、接近方向與危險區域輸出風險等級。
- 以表格方式輸出每個時間點的最高風險人車配對。

## 限制與未來延伸

目前版本重點在 C++ 判斷邏輯，因此尚未直接讀取攝影機影像，也尚未整合即時 OpenCV 偵測流程。未來可將 `cars.xml` 與 OpenCV 影像辨識串接，讓程式從實際影像取得車輛與行人的偵測框，再使用本專案已完成的風險判斷邏輯輸出警示結果。
