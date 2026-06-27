#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <limits>
#include <iostream>
#include <string>

//十二位元組浮點運算結構
struct float3
{
    float x;
    float y;
    float z;
};

// 3D vector helper functions
inline float3 operator-(const float3& a, const float3& b) {
    return float3{a.x - b.x, a.y - b.y, a.z - b.z};
}
inline float3 operator+(const float3& a, const float3& b) {
    return float3{a.x + b.x, a.y + b.y, a.z + b.z};
}
inline float3 operator*(const float3& a, float f) {
    return float3{a.x * f, a.y * f, a.z * f};
}
inline float dot(const float3& a, const float3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline float3 cross(const float3& a, const float3& b) {
    return float3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
inline float3 normalize(const float3& v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len == 0.0f) return v;
    return float3{v.x / len, v.y / len, v.z / len};
}

// 4x4 Matrix representation - Row-Major Layout
struct Matrix4x4 {
    float m[4][4];
    
    static Matrix4x4 identity() {
        Matrix4x4 mat = {{{0}}};
        for (int i = 0; i < 4; ++i) mat.m[i][i] = 1.0f;
        return mat;
    }
};

struct Vec4 {
    float x, y, z, w;
};

// Row-Major Matrix-Vector Multiplication (Sequential Access)
inline Vec4 multiply(const Matrix4x4& M, const Vec4& V) {
    Vec4 result;
    result.x = M.m[0][0] * V.x + M.m[0][1] * V.y + M.m[0][2] * V.z + M.m[0][3] * V.w;
    result.y = M.m[1][0] * V.x + M.m[1][1] * V.y + M.m[1][2] * V.z + M.m[1][3] * V.w;
    result.z = M.m[2][0] * V.x + M.m[2][1] * V.y + M.m[2][2] * V.z + M.m[2][3] * V.w;
    result.w = M.m[3][0] * V.x + M.m[3][1] * V.y + M.m[3][2] * V.z + M.m[3][3] * V.w;
    return result;
}

// Matrix-Matrix Multiplication (Row-Major Row-by-Column)
inline Matrix4x4 multiply_matrix(const Matrix4x4& A, const Matrix4x4& B) {
    Matrix4x4 C = {{{0}}};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            C.m[i][j] = A.m[i][0] * B.m[0][j] +
                        A.m[i][1] * B.m[1][j] +
                        A.m[i][2] * B.m[2][j] +
                        A.m[i][3] * B.m[3][j];
        }
    }
    return C;
}

inline Matrix4x4 lookAt(float3 eye, float3 center, float3 up) {
    float3 z = normalize(eye - center);
    float3 x = normalize(cross(up, z));
    float3 y = normalize(cross(z, x));
    
    Matrix4x4 Minv = Matrix4x4::identity();
    Matrix4x4 Tr = Matrix4x4::identity();
    
    for (int i = 0; i < 3; ++i) {
        // Set rotation rows
        Minv.m[0][i] = (i == 0) ? x.x : ((i == 1) ? x.y : x.z);
        Minv.m[1][i] = (i == 0) ? y.x : ((i == 1) ? y.y : y.z);
        Minv.m[2][i] = (i == 0) ? z.x : ((i == 1) ? z.y : z.z);
    }
    
    Tr.m[0][3] = -eye.x;
    Tr.m[1][3] = -eye.y;
    Tr.m[2][3] = -eye.z;
    
    return multiply_matrix(Minv, Tr);
}

inline Matrix4x4 projection(float coeff) {
    Matrix4x4 P = Matrix4x4::identity();
    P.m[3][2] = coeff;
    return P;
}

inline Matrix4x4 viewport(int x, int y, int w, int h) {
    Matrix4x4 m = Matrix4x4::identity();
    m.m[0][0] = w / 2.f;
    m.m[0][3] = x + w / 2.f;
    m.m[1][1] = -h / 2.f;
    m.m[1][3] = y + h / 2.f;
    m.m[2][2] = 1.0f;
    m.m[2][3] = 0.0f;
    return m;
}

//八位元組浮點運算結構
struct float2
{
    float x;
    float y;
};

//十二位元組整數運算結構
struct int3
{
    int x;
    int y;
    int z;
};

//八位元組整數運算結構
struct int2
{
    int x;
    int y;
};

struct uint2
{
    unsigned int x;
    unsigned int y;
};

struct color
{
    unsigned char r, g, b, a; // 補齊 4 bytes 滿足記憶體對齊，完美支援 SIMD 與快取
};

struct FaceVertex {
    int v_idx;   // 頂點索引 (0-based)
    int vt_idx;  // UV 紋理索引
    int vn_idx;  // 法線索引
};

std::vector<float3> Model_Vertices;
std::vector<FaceVertex> Model_Faces;

//畫布大小
const int width = 800;
const int height = 800;

std::vector<color> canva(width * height, color{0, 0, 0, 255}); // 初始化畫布為黑色
std::vector<float> zbuffer(width * height, -std::numeric_limits<float>::max());



//設置像素顏色的函式
void SetPixel(int2 pos, color c){
        // 安全螢幕裁剪 (Bounds Culling)
        if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) return;
        int index = pos.y * width + pos.x; // 計算像素在畫布中的索引    
        canva[index] = c;
    }
//時間步進算法開始
    //OverDraw 線段繪製函式
    void line (float2 Start_Vertex, float2 End_Vertex ){
        int X = 1;
        int Y = 1;
        int dx = End_Vertex.x - Start_Vertex.x; // 水平向量
        int dy = End_Vertex.y - Start_Vertex.y; // 垂直向量
        float  m = dy / dx; // 斜率
        float length =  End_Vertex.x - Start_Vertex.x; // 線段長度 X軸的距離
            for(float t=0; t<length; t+=0.1f){
                int x = (int)(Start_Vertex.x + X * t+0.5f);//起點的水平向量加上現段長度乘以步進時間 +0.5f是為了四捨五入到最近的整數
                int y = (int)(Start_Vertex.y + Y * m * t+0.5f);//起點的垂直向量加上現段長度乘以步進時加入加入 m是讓布進時間乘上斜率 追上X軸的不盡速度 +0.5f是為了四捨五入到最近的整數 
                SetPixel(int2{x, y}, color{255, 255, 255}); // 設置像素為白色 根據步進到的時間做點亮
            }
        }
//時間步進算法結束

//我的步進算法開始
    void line2(int2 Start_Vertex, int2 End_Vertex){
        int X_Vector = End_Vertex.x - Start_Vertex.x; // 水平向量
        int Y_Vector = End_Vertex.y - Start_Vertex.y; // 垂直向量
        //X = 1 代表水平向量的主軸 以水平向量為基準來決定步進的時間
        //Y = 1 代表垂直向量的主軸 以垂直向量為基準來決定步進的時間
        int maskX = X_Vector >> 31; // 取得水平向量的符號位元，負數為-1，非負數為0
        int maskY = Y_Vector >> 31; // 取得垂直向量的符號位元，負數為-1，非負數為0
        int lengthX = (X_Vector^maskX) - maskX; // 根據水平向量的符號計算水平向量的長度
        int lengthY = (Y_Vector^maskY) - maskY; // 根據垂直向量的符號計算垂直向量的長度
        int diff = lengthX - lengthY; // 水平向量與垂直向量的差距
        int mask = diff >> 31; // 取得差距的符號位元，負數為-1，非負數為0
        int length = (lengthX & ~mask) | (lengthY & mask); // 根據差距的符號選擇水平向量或垂直向量作為線段長度 
        int Minor_Length = (lengthY & ~mask) | (lengthX & mask);//把length的位元設計反過來就可以得到我們的副軸長度
        int frequency = (Minor_Length << 16) / length; // 計算步進頻率
        int Start_Vertex_Minor = (Start_Vertex.y & ~mask) | (Start_Vertex.x & mask);
        int minor_phase_accumulator = (Start_Vertex_Minor << 16) + 32768; // 初始相位累加器，加入32768是為了實現四捨五入
        int Start_Vertex_Major = (Start_Vertex.x & ~mask) | (Start_Vertex.y & mask);// 根據主軸選擇起點的主要座標
        for(int t=0; t<length; t++){
          int current_minor = minor_phase_accumulator >> 16;// 取得當前的副軸座標
          int current_major = Start_Vertex_Major + t;// 計算當前的主軸座標
          int x = (current_major & ~mask) | (current_minor & mask); // 根據主軸選擇當前的x座標
          int y = (current_major & mask) | (current_minor & ~mask); // 根據主軸選擇當前的y座標
          SetPixel(int2{x, y}, color{255, 255, 255}); // 設置像素為白色 根據步進到的時間做點亮
          minor_phase_accumulator += frequency; // 更新相位累加器
        }
    }
//我的步進算法結束 

void Triangle_ragister(int2 v0, int2 v1, int2 v2){
    // 1. 捨棄指標，直上暫存器 Value Swap，徹底擺脫記憶體指標束縛
    if (v0.y > v1.y) std::swap(v0, v1);
    if (v0.y > v2.y) std::swap(v0, v2);
    if (v1.y > v2.y) std::swap(v1, v2);

    int botX = v0.x, botY = v0.y;
    int midX = v1.x, midY = v1.y;
    int topX = v2.x, topY = v2.y;

    // Y 軸全局裁剪 (Screen Culling)，若三角形完全出界則不繪製
    if (botY >= height || topY < 0) return; 

    // 2. 修復 DDA 數學邏輯（拿掉荒謬的 abs 取絕對值，保留向量符號）
    int long_dy = topY - botY;
    int long_dx = topX - botX;
    int long_frequency = (long_dy == 0) ? 0 : (long_dx << 16) / long_dy;
    int long_phase_accumulator = (botX << 16) + 32768;

    int short_dy1 = midY - botY;
    int short_dx1 = midX - botX;
    int short_frequency1 = (short_dy1 == 0) ? 0 : (short_dx1 << 16) / short_dy1;
    int short_phase_accumulator = (botX << 16) + 32768;

    // 限制 Y 軸不越界
    int y_start = std::max(0, botY);
    int y_end   = std::min(height - 1, midY);

    // 為了應對 y_start > botY (底部被裁剪) 的情況，相位器必須提前前進：
    if (y_start > botY) {
        int steps = y_start - botY;
        long_phase_accumulator += long_frequency * steps;
        short_phase_accumulator += short_frequency1 * steps;
    }

    // -------------------------------------------------------------------------
    // 🚂 下半場填滿 (從 botY 爬到 midY) ── 平底三角形
    // -------------------------------------------------------------------------
    for (int y = y_start; y <= y_end; y++) {
        int x1 = long_phase_accumulator >> 16;
        int x2 = short_phase_accumulator >> 16;
        if (x1 > x2) std::swap(x1, x2);

        // X 軸安全裁剪 (X Culling)
        int x_start = std::max(0, x1);
        int x_end   = std::min(width - 1, x2);

        // 緩存友善 (Cache Friendly) 的內迴圈掃描，避免重複乘法
        int row_offset = y * width;
        for (int x = x_start; x <= x_end; x++) {
            canva[row_offset + x] = color{255, 255, 255, 255}; 
        }

        long_phase_accumulator  += long_frequency;
        short_phase_accumulator += short_frequency1;
    }

    // -------------------------------------------------------------------------
    // 🚂 上半場填滿 (從 midY + 1 爬到 topY) ── 平頂三角形
    // -------------------------------------------------------------------------
    int short_dy2 = topY - midY;
    int short_dx2 = topX - midX;
    int short_frequency2 = (short_dy2 == 0) ? 0 : (short_dx2 << 16) / short_dy2;
    
    int y_start2 = std::max(0, midY + 1);
    int y_end2   = std::min(height - 1, topY);

    // 短邊軌道起跑點與相位補償
    short_phase_accumulator = (midX << 16) + 32768; 
    if (y_start2 > midY + 1) {
        short_phase_accumulator += short_frequency2 * (y_start2 - (midY + 1));
    }
    
    // 長邊軌道強制同步至 y_start2，避免迴圈斷層導致的相位誤差
    int long_phase_accumulator_upper = (botX << 16) + 32768 + long_frequency * (y_start2 - botY);

    for (int y = y_start2; y <= y_end2; y++) {
        int x1 = long_phase_accumulator_upper >> 16;
        int x2 = short_phase_accumulator >> 16;
        if (x1 > x2) std::swap(x1, x2);

        int x_start = std::max(0, x1);
        int x_end   = std::min(width - 1, x2);

        int row_offset = y * width;
        for (int x = x_start; x <= x_end; x++) {
            canva[row_offset + x] = color{255, 255, 255, 255};
        }

        long_phase_accumulator_upper += long_frequency;
        short_phase_accumulator += short_frequency2;
    }
}

// 重心座標光柵化與顏色插值管線 (Barycentric Rasterization & Color Interpolation)
// 
// 數學原理說明：
// 我們欲解平面上某點 P(x, y) 對應三角形三個頂點 A, B, C 的重心座標 alpha, beta, gamma，滿足：
//   alpha * Ax + beta * Bx + gamma * Cx = x
//   alpha * Ay + beta * By + gamma * Cy = y
//   alpha + beta + gamma = 1
//
// 寫成矩陣形式：
//   [ Ax  Bx  Cx ]   [ alpha ]   [ x ]
//   [ Ay  By  Cy ] * [ beta  ] = [ y ]
//   [  1   1   1 ]   [ gamma ]   [ 1 ]
//
// 記此 3x3 矩陣為 M。求其逆矩陣 M^-1：
//   M^-1 = (1 / det(M)) * adj(M)
//
// 其中伴隨矩陣 adj(M) 為餘因子矩陣的轉置：
//   adj(M) = [ By-Cy  Cx-Bx  Bx*Cy-Cx*By ]
//            [ Cy-Ay  Ax-Cx  Cx*Ay-Ax*Cy ]
//            [ Ay-By  Bx-Ax  Ax*By-Bx*Ay ]
//
// 因此：
//   [ alpha ]          [ (By-Cy)x + (Cx-Bx)y + (Bx*Cy-Cx*By) ]
//   [ beta  ] = 1/D *  [ (Cy-Ay)x + (Ax-Cx)y + (Cx*Ay-Ax*Cy) ]
//   [ gamma ]          [ (Ay-By)x + (Bx-Ax)y + (Ax*By-Bx*Ay) ]
//
// 這正是克萊姆法則（Cramer's Rule）解聯立方程式之結果。
// det(M) 即為三角形有向面積的兩倍 (Oriented Area * 2)，記為 D。
// 令邊緣函數 w0(x, y)、w1(x, y)、w2(x, y) 對應上述三項分子：
//   w0(x, y) = (By-Cy)x + (Cx-Bx)y + (Bx*Cy-Cx*By)  (對應邊 BC，其值為 gamma 權重分子的幾何表示)
//   w1(x, y) = (Cy-Ay)x + (Ax-Cx)y + (Cx*Ay-Ax*Cy)  (對應邊 CA，其值為 alpha 權重分子的幾何表示)
//   w2(x, y) = (Ay-By)x + (Bx-Ax)y + (Ax*By-Bx*Ay)  (對應邊 AB，其值為 beta 權重分子的幾何表示)
//
// 為了避免重複乘法並發揮硬體極限，我們利用邊緣函數的線性特徵，在外迴圈初始化起點後，
// 內迴圈步進 x 時直接以常數增量進行純整數加法累加（Incremental Update）：
//   w0(x + 1, y) = w0(x, y) + a0, 其中 a0 = By - Cy
//   w0(x, y + 1) = w0(x, y) + b0, 其中 b0 = Cx - Bx
//
// 邊界判定嚴格實作現代 GPU 規格之「左上規則 (Top-Left Rule)」，防止像素重疊與漏縫：
//   對於一條邊，若其滿足：
//   1. 它是左邊 (Left Edge)：非水平且朝上的邊 (dy < 0，在 y 軸向下為正的螢幕空間下)
//   2. 它是上邊 (Top Edge)：水平且朝左的邊 (dy == 0 且 dx < 0)
//   則該邊界上的點判定為在三角形內。
//   為與 winding order (CCW) 一致，且 edge function E(x,y) = a*x + b*y + c 在內部為正，
//   我們定義 Top-Left 條件為： (a > 0) || (a == 0 && b > 0)
//
void RasterizeTriangleBarycentric(float3 v0, float3 v1, float3 v2, color c0, color c1, color c2) {
    // 1. 計算行列式 D (雙倍有向面積)
    float D = (v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y);
    if (std::abs(D) < 1e-5f) return; // 共線或退化三角形，直接捨棄

    // 2. 確保頂點為逆時針 (CCW) 排列。若 D < 0，則順時針，藉由交換 v1, v2 與 c1, c2 轉為 CCW
    if (D < 0) {
        std::swap(v1, v2);
        std::swap(c1, c2);
        D = -D;
    }

    // 3. 定義邊緣函數係數 (a, b, c) 與 Top-Left 規則
    // Edge 0: v1 -> v2 (BC) -> 決定 w0 (對應 v2/C 的權重)
    float a0 = v1.y - v2.y;
    float b0 = v2.x - v1.x;
    float c0_const = v1.x * v2.y - v2.x * v1.y;
    bool is_top_left_0 = (a0 > 0) || (a0 == 0 && b0 > 0);

    // Edge 1: v2 -> v0 (CA) -> 決定 w1 (對應 v0/A 的權重)
    float a1 = v2.y - v0.y;
    float b1 = v0.x - v2.x;
    float c1_const = v2.x * v0.y - v0.x * v2.y;
    bool is_top_left_1 = (a1 > 0) || (a1 == 0 && b1 > 0);

    // Edge 2: v0 -> v1 (AB) -> 決定 w2 (對應 v1/B 的權重)
    float a2 = v0.y - v1.y;
    float b2 = v1.x - v0.x;
    float c2_const = v0.x * v1.y - v1.x * v0.y;
    bool is_top_left_2 = (a2 > 0) || (a2 == 0 && b2 > 0);

    // 4. 計算包圍盒 (Bounding Box) 並進行螢幕裁剪
    int x_min = std::max(0, (int)(std::min({v0.x, v1.x, v2.x}) + 0.5f));
    int x_max = std::min(width - 1, (int)(std::max({v0.x, v1.x, v2.x}) + 0.5f));
    int y_min = std::max(0, (int)(std::min({v0.y, v1.y, v2.y}) + 0.5f));
    int y_max = std::min(height - 1, (int)(std::max({v0.y, v1.y, v2.y}) + 0.5f));

    // 5. 差分步進初始化 (以 x_min, y_min 為起點)
    float w0_row = a0 * x_min + b0 * y_min + c0_const;
    float w1_row = a1 * x_min + b1 * y_min + c1_const;
    float w2_row = a2 * x_min + b2 * y_min + c2_const;

    float inv_D = 1.0f / D;

    // 6. 雙重掃描線迴圈
    for (int y = y_min; y <= y_max; ++y) {
        float w0 = w0_row;
        float w1 = w1_row;
        float w2 = w2_row;
        int row_offset = y * width; // 行偏移快取優化 (Row Offset Cache)

        uint32_t* canva_u32 = reinterpret_cast<uint32_t*>(canva.data());

        for (int x = x_min; x <= x_max; ++x) {
            // 嚴格套用左上規則進行邊界判定
            bool inside0 = (w0 > 0) || (w0 == 0 && is_top_left_0);
            bool inside1 = (w1 > 0) || (w1 == 0 && is_top_left_1);
            bool inside2 = (w2 > 0) || (w2 == 0 && is_top_left_2);

            if (inside0 && inside1 && inside2) {
                // 重心座標權重計算：w0 對應 v2，w1 對應 v0，w2 對應 v1
                float l0 = w1 * inv_D; // v0 權重
                float l1 = w2 * inv_D; // v1 權重
                float l2 = w0 * inv_D; // v2 權重

                // 插值深度 Z
                float z = l0 * v0.z + l1 * v1.z + l2 * v2.z;

                int idx = row_offset + x;
                float old_z = zbuffer[idx];
                bool pass = z > old_z;
                
                zbuffer[idx] = pass ? z : old_z;

                // 顏色線性插值與安全四捨五入
                color pixel_color;
                pixel_color.r = (unsigned char)(l0 * c0.r + l1 * c1.r + l2 * c2.r + 0.5f);
                pixel_color.g = (unsigned char)(l0 * c0.g + l1 * c1.g + l2 * c2.g + 0.5f);
                pixel_color.b = (unsigned char)(l0 * c0.b + l1 * c1.b + l2 * c2.b + 0.5f);
                pixel_color.a = 255;

                uint32_t color_u32 = *reinterpret_cast<uint32_t*>(&pixel_color);
                canva_u32[idx] = pass ? color_u32 : canva_u32[idx];
            }

            // 內迴圈步進 x：常數整數增量，無乘法
            w0 += a0;
            w1 += a1;
            w2 += a2;
        }

        // 外迴圈步進 y：常數整數增量，無乘法
        w0_row += b0;
        w1_row += b1;
        w2_row += b2;
    }
}

// 動態檔名輸出功能，防止覆蓋歷史圖片
void Out_imagefile_custom(const std::string& filename) {
    std::ofstream out_file(filename); 
    out_file << "P3\n" << width << " " << height << "\n255\n";

    for (int i = 0; i < width * height; i++) {
        out_file << (int)canva[i].r << " " 
                 << (int)canva[i].g << " " 
                 << (int)canva[i].b << "\n";
    }
    out_file.close();
}

inline bool Out_tga(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) return false;
    
    unsigned char header[18] = {0};
    header[2] = 2; // uncompressed RGB
    header[12] = width & 0xFF;
    header[13] = (width >> 8) & 0xFF;
    header[14] = height & 0xFF;
    header[15] = (height >> 8) & 0xFF;
    header[16] = 24; // 24-bit BGR
    header[17] = 0x20; // top-to-bottom, left-to-right
    
    out.write(reinterpret_cast<char*>(header), 18);
    for (int i = 0; i < width * height; i++) {
        out.put(canva[i].b);
        out.put(canva[i].g);
        out.put(canva[i].r);
    }
    out.close();
    return true;
}

inline bool Out_bmp(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) return false;
    
    unsigned char header[54] = {0};
    header[0] = 'B'; header[1] = 'M';
    uint32_t file_size = 54 + width * height * 3;
    header[2] = file_size & 0xFF;
    header[3] = (file_size >> 8) & 0xFF;
    header[4] = (file_size >> 16) & 0xFF;
    header[5] = (file_size >> 24) & 0xFF;
    
    header[10] = 54; // data offset
    header[14] = 40; // info header size
    header[18] = width & 0xFF;
    header[19] = (width >> 8) & 0xFF;
    header[20] = (width >> 16) & 0xFF;
    header[21] = (width >> 24) & 0xFF;
    
    int32_t bmp_height = -height; // negative height for top-to-bottom
    header[22] = bmp_height & 0xFF;
    header[23] = (bmp_height >> 8) & 0xFF;
    header[24] = (bmp_height >> 16) & 0xFF;
    header[25] = (bmp_height >> 24) & 0xFF;
    
    header[26] = 1; // planes
    header[28] = 24; // bits per pixel
    
    out.write(reinterpret_cast<char*>(header), 54);
    
    int padding = (4 - (width * 3) % 4) % 4;
    unsigned char pad[3] = {0};
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            out.put(canva[idx].b);
            out.put(canva[idx].g);
            out.put(canva[idx].r);
        }
        if (padding > 0) {
            out.write(reinterpret_cast<char*>(pad), padding);
        }
    }
    out.close();
    return true;
}
     


    
// 以純整數格式將記憶體內容噴進硬碟
void Out_imagefile(){
        std::ofstream out_file("text_image_02.ppm"); 
        // 寫入標頭防禦陣列
        out_file << "P3\n" << width << " " << height << "\n255\n";

        for (int i = 0; i < width * height; i++) {
            out_file << (int)canva[i].r << " " 
                     << (int)canva[i].g << " " 
                     << (int)canva[i].b << "\n";
            }
            out_file.close(); // 釋放磁碟控制權
    }  
    
bool Load_Obj_AOS_Custom_Geometry(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "[-] 錯誤：無法開啟模型檔案 " << filename << " !!! " << std::endl;
        return false;
    }

    std::string line;
    // 使用 std::getline 更安全地讀取每一行
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        // 🎯 A. 解析幾何頂點 (v x y z)
        if (!line.compare(0, 2, "v ")) {
            float x, y, z;
            // 確保 sscanf 真的讀到 3 個浮點數才寫入
            if (sscanf(line.c_str(), "v %f %f %f", &x, &y, &z) == 3) {
                Model_Vertices.push_back({ x, y, z }); 
            }
        }
        // 🎯 B. 解析面拓撲 (f v/vt/vn ...)
        else if (!line.compare(0, 2, "f ")) {
            int v[4], vt[4], vn[4];
            
            // 嘗試匹配四邊形 (Quad) ── 12 個變數
            int count = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                               &v[0], &vt[0], &vn[0],
                               &v[1], &vt[1], &vn[1],
                               &v[2], &vt[2], &vn[2],
                               &v[3], &vt[3], &vn[3]);

            if (count == 12) {
                // 🟢 情況一：四邊形，拆成連續的 FaceVertex
                for (int i = 0; i < 4; ++i) {
                    Model_Faces.push_back({ v[i] - 1, vt[i] - 1, vn[i] - 1 });
                }
            }
            else {
                // 🟡 情況二：精確匹配標準三邊形 (Triangle) ── 9 個變數
                count = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                               &v[0], &vt[0], &vn[0],
                               &v[1], &vt[1], &vn[1],
                               &v[2], &vt[2], &vn[2]);
                if (count == 9) {
                    for (int i = 0; i < 3; ++i) {
                        Model_Faces.push_back({ v[i] - 1, vt[i] - 1, vn[i] - 1 });
                    }
                }
            }

            // 🔒 如果成功解析，銲接一尊 {-1, -1, -1} 斷面記號
            if (count == 12 || count == 9) {
                Model_Faces.push_back({ -1, -1, -1 });
            }
        }
    }
    
    in.close();
    
    std::cout << "[+] 成功！頂點陣列(float3)數量: " << Model_Vertices.size() 
              << "，打平後的面索引(FaceVertex)數量: " << Model_Faces.size() << std::endl;
    
    return true;
}    
#endif // GEOMETRY_H 