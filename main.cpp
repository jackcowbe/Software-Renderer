#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include "Geometry.h"

// Define camera parameters
const float3 eye = {1.0f, 1.0f, 3.0f};
const float3 center = {0.0f, 0.0f, 0.0f};
const float3 up = {0.0f, 1.0f, 0.0f};

Matrix4x4 PV; // Product of Projection and ModelView matrices

void render_triangle(const FaceVertex& f0, const FaceVertex& f1, const FaceVertex& f2) {
    float3 v0 = Model_Vertices[f0.v_idx];
    float3 v1 = Model_Vertices[f1.v_idx];
    float3 v2 = Model_Vertices[f2.v_idx];

    // Compute flat shading normal and light intensity in world space
    float3 n = normalize(cross(v1 - v0, v2 - v0));
    float3 light_dir = normalize(eye - center);
    float intensity = dot(n, light_dir);
    
    if (intensity > 0.0f) {
        // Multiply vertices by projection-modelview matrix using Row-Major sequential memory access
        Vec4 v0_clip = multiply(PV, Vec4{v0.x, v0.y, v0.z, 1.0f});
        Vec4 v1_clip = multiply(PV, Vec4{v1.x, v1.y, v1.z, 1.0f});
        Vec4 v2_clip = multiply(PV, Vec4{v2.x, v2.y, v2.z, 1.0f});

        // Hand-coded perspective division (Perspective Divide)
        // Clear, manual, explicit implementation of x = x / w; y = y / w; z = z / w;
        float x0 = v0_clip.x / v0_clip.w;
        float y0 = v0_clip.y / v0_clip.w;
        float z0 = v0_clip.z / v0_clip.w;

        float x1 = v1_clip.x / v1_clip.w;
        float y1 = v1_clip.y / v1_clip.w;
        float z1 = v1_clip.z / v1_clip.w;

        float x2 = v2_clip.x / v2_clip.w;
        float y2 = v2_clip.y / v2_clip.w;
        float z2 = v2_clip.z / v2_clip.w;

        // Viewport mapping from NDC [-1, 1] to screen coordinates
        float3 s0, s1, s2;
        s0.x = (width / 2.0f) * x0 + (width / 2.0f);
        s0.y = (-height / 2.0f) * y0 + (height / 2.0f);
        s0.z = z0;

        s1.x = (width / 2.0f) * x1 + (width / 2.0f);
        s1.y = (-height / 2.0f) * y1 + (height / 2.0f);
        s1.z = z1;

        s2.x = (width / 2.0f) * x2 + (width / 2.0f);
        s2.y = (-height / 2.0f) * y2 + (height / 2.0f);
        s2.z = z2;

        unsigned char val = (unsigned char)(intensity * 255.0f + 0.5f);
        color col = {val, val, val, 255};

        // Render using the original barycentric rasterizer with Z-buffer and top-left rules
        RasterizeTriangleBarycentric(s0, s1, s2, col, col, col);
    }
}

int main() {
    std::cout << "[+] Starting 3D Camera Rendering Pipeline..." << std::endl;
    
    // Load 3D model
    std::string model_path = "african_head.obj";
    if (!Load_Obj_AOS_Custom_Geometry(model_path)) {
        std::cerr << "[-] Failed to load model: " << model_path << std::endl;
        return -1;
    }

    // Initialize canvas to black and Z-buffer to -infinity
    std::fill(canva.begin(), canva.end(), color{0, 0, 0, 255});
    std::fill(zbuffer.begin(), zbuffer.end(), -std::numeric_limits<float>::max());

    // Build transformation matrices
    Matrix4x4 ModelView = lookAt(eye, center, up);
    
    // Distance from camera eye to center (target)
    float dist = std::sqrt(dot(eye - center, eye - center));
    Matrix4x4 Projection = projection(-1.0f / dist);

    // Combine Projection and ModelView
    PV = multiply_matrix(Projection, ModelView);

    // Render faces (triangles & quads)
    std::vector<FaceVertex> face;
    for (size_t i = 0; i < Model_Faces.size(); i++) {
        if (Model_Faces[i].v_idx == -1) {
            if (face.size() == 3) {
                render_triangle(face[0], face[1], face[2]);
            } else if (face.size() == 4) {
                render_triangle(face[0], face[1], face[2]);
                render_triangle(face[0], face[2], face[3]);
            }
            face.clear();
        } else {
            face.push_back(Model_Faces[i]);
        }
    }

    // Output to TGA file
    std::string output_filename = "output.tga";
    if (Out_tga(output_filename)) {
        std::cout << "[+] Render complete. Output written to " << output_filename << std::endl;
    } else {
        std::cerr << "[-] Error writing TGA file." << std::endl;
    }

    // Also output BMP for markdown display
    Out_bmp("output.bmp");

    return 0;
}
