# Software-Renderer (CPU 軟體光柵化渲染器)

這是一個以純 C++ 實現的輕量級 CPU 軟體光柵化渲染器，不依賴任何第三方圖形或幾何庫。本專案從底層開始實作完整的三維圖形渲染管線，包含相機視圖變換、透視投影、手撕透視除法、遵守左上規則（Top-Left Rule）與 CCW 頂點排列的重心座標光柵化，以及快取友善的無分支（Branchless）Z-Buffer 深度測試。

---

## 🛠️ 技術特點與架構設計

### 1. 齊次座標變換與 Row-Major 記憶體對齊
- 實作了 $4 \times 4$ 矩陣與 3D 向量運算，包含 `lookAt`（視圖矩陣）、`projection`（透視投影矩陣）與 `viewport`（視口變換矩陣）。
- **快取局部性優化**：所有頂點的齊次座標矩陣乘法均嚴格遵循 C++ 的 **Row-Major（行主序）連續記憶體定址**。矩陣乘法以列（Row）連續讀取，消除了跨排跳躍（Cross-bank jump）與跨 Cache Line 讀取引起的延遲，最大化 CPU 快取命中率。

### 2. 手撕透視除法 (Perspective Divide)
- 在齊次剪裁空間變換後，顯式且人肉手寫透視除法：
  ```cpp
  float x0 = v0_clip.x / v0_clip.w;
  float y0 = v0_clip.y / v0_clip.w;
  float z0 = v0_clip.z / v0_clip.w;
  ```
  確保頂點坐標正確投影到 Normalized Device Coordinates (NDC) 空間。

### 3. 重心座標光柵化與頂點插值
- 實作了基於重心座標的三角形填充演算法。
- **左上規則（Top-Left Rule）**：嚴格實作標準 GPU 規格的左上邊界判定，防止相鄰三角形邊緣像素重疊或產生縫隙，支援 Counter-Clockwise (CCW) 逆時針頂點排序判定。
- **深度插值與防畸變**：利用重心座標權重（$l_0, l_1, l_2$）對頂點深度值 $z$ 進行精確線性插值：
  ```cpp
  float z = l0 * v0.z + l1 * v1.z + l2 * v2.z;
  ```

### 4. 無分支（Branchless）Z-Buffer 與零拷貝寫入
- 為了避免 CPU 分支預測錯誤（Branch Misprediction）帶來的高昂代價，在最內層像素填充迴圈中，深度測試與色彩緩衝寫入完全消除 `if-else` 分支。
- 使用三元運算子使編譯器在 `/O2` 最佳化下直接生成彙編級的條件傳送指令（`cmov`），並透過 `uint32_t` 進行對齊以提升記憶體寫入效率：
  ```cpp
  int idx = row_offset + x;
  float old_z = zbuffer[idx];
  bool pass = z > old_z;
  
  zbuffer[idx] = pass ? z : old_z;
  canva_u32[idx] = pass ? color_u32 : canva_u32[idx];
  ```

### 5. 多格式輸出
- 支援輸出未壓縮的 24-bit BGR 格式 Truevision TGA 檔案（使用 `0x20` 頂部到左側掃描配置與畫布定址完全對齊）。
- 支援 BMP 格式輸出，便於跨平台預覽。

---

## 📂 專案檔案結構

- **`Geometry.h`**：核心數學庫（向量、矩陣、變換）與 3D Barycentric 光柵化器、TGA/BMP 檔案輸出實作。
- **`main.cpp`**：主入口點，實作 OBJ 模型載入、相機矩陣組裝、頂點著色、光柵化渲染管線與 Flat Shading（平面著色）強度計算。
- **`build.bat`**：MSVC 極限編譯與建置指令碼。
- **`african_head.obj`**：預設測試用的 3D 頭部模型檔案。

---

## 🚀 建置與編譯指南

本專案配置為在 Windows 環境下使用 MSVC 進行編譯與最佳化。

### 編譯步驟
1. 開啟 **Developer Command Prompt for VS 2022**。
2. 進入專案目錄並執行 `build.bat`：
   ```cmd
   build.bat
   ```

### 最佳化編譯選項
`build.bat` 指令碼會自動配置 MSVC環境，並使用極限最佳化選項 `/O2` 進行編譯：
```cmd
cl /EHsc /O2 main.cpp
```
編譯成功後，程式會自動執行並加載 `african_head.obj`，在目錄下生成 `output.tga` 與可預覽的 `output.bmp`。
