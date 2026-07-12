#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>
#include <tex3ds.h>
#include <string.h>
#include "vshader_shbin.h"
#include "texture_t3x.h"

#define CLEAR_COLOR 0x68B0D8FF

#define DISPLAY_TRANSFER_FLAGS (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

typedef struct {
    float position[3];
    float texcoord[2];
    float normal[3];
} vertex;

enum FACE { FACE_PZ, FACE_MZ, FACE_PX, FACE_MX, FACE_PY, FACE_MY };

#define UV_X (1.0f / 64.0f)
#define UV_Y (1.0f / 4.0f)

static const vertex cube_faces[] = {
    // First face (PZ)
    // First triangle
    {{-0.5f, -0.5f, +0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {0.0f, 0.0f, +1.0f}},
    {{+0.5f, -0.5f, +0.5f}, {UV_X, 0.0f + 3 * UV_Y}, {0.0f, 0.0f, +1.0f}},
    {{+0.5f, +0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {0.0f, 0.0f, +1.0f}},
    // Second triangle
    {{+0.5f, +0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {0.0f, 0.0f, +1.0f}},
    {{-0.5f, +0.5f, +0.5f}, {0.0f, UV_Y + 3 * UV_Y}, {0.0f, 0.0f, +1.0f}},
    {{-0.5f, -0.5f, +0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {0.0f, 0.0f, +1.0f}},

    // Second face (MZ)
    // First triangle
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, +0.5f, -0.5f}, {UV_X, 0.0f + 3 * UV_Y}, {0.0f, 0.0f, -1.0f}},
    {{+0.5f, +0.5f, -0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {0.0f, 0.0f, -1.0f}},
    // Second triangle
    {{+0.5f, +0.5f, -0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {0.0f, 0.0f, -1.0f}},
    {{+0.5f, -0.5f, -0.5f}, {0.0f, UV_Y + 3 * UV_Y}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {0.0f, 0.0f, -1.0f}},

    // Third face (PX)
    // First triangle
    {{+0.5f, -0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {+1.0f, 0.0f, 0.0f}},
    {{+0.5f, +0.5f, -0.5f}, {UV_X, 0.0f + 3 * UV_Y}, {+1.0f, 0.0f, 0.0f}},
    {{+0.5f, +0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {+1.0f, 0.0f, 0.0f}},
    // Second triangle
    {{+0.5f, +0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {+1.0f, 0.0f, 0.0f}},
    {{+0.5f, -0.5f, +0.5f}, {0.0f, UV_Y + 3 * UV_Y}, {+1.0f, 0.0f, 0.0f}},
    {{+0.5f, -0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {+1.0f, 0.0f, 0.0f}},

    // Fourth face (MX)
    // First triangle
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, +0.5f}, {UV_X, 0.0f + 3 * UV_Y}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, +0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {-1.0f, 0.0f, 0.0f}},
    // Second triangle
    {{-0.5f, +0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, +0.5f, -0.5f}, {0.0f, UV_Y + 3 * UV_Y}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {-1.0f, 0.0f, 0.0f}},

    // Fifth face (PY)
    // First triangle
    {{-0.5f, +0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {0.0f, +1.0f, 0.0f}},
    {{-0.5f, +0.5f, +0.5f}, {UV_X, 0.0f + 3 * UV_Y}, {0.0f, +1.0f, 0.0f}},
    {{+0.5f, +0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {0.0f, +1.0f, 0.0f}},
    // Second triangle
    {{+0.5f, +0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {0.0f, +1.0f, 0.0f}},
    {{+0.5f, +0.5f, -0.5f}, {0.0f, UV_Y + 3 * UV_Y}, {0.0f, +1.0f, 0.0f}},
    {{-0.5f, +0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {0.0f, +1.0f, 0.0f}},

    // Sixth face (MY)
    // First triangle
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {0.0f, -1.0f, 0.0f}},
    {{+0.5f, -0.5f, -0.5f}, {UV_X, 0.0f + 3 * UV_Y}, {0.0f, -1.0f, 0.0f}},
    {{+0.5f, -0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {0.0f, -1.0f, 0.0f}},
    // Second triangle
    {{+0.5f, -0.5f, +0.5f}, {UV_X, UV_Y + 3 * UV_Y}, {0.0f, -1.0f, 0.0f}},
    {{-0.5f, -0.5f, +0.5f}, {0.0f, UV_Y + 3 * UV_Y}, {0.0f, -1.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f + 3 * UV_Y}, {0.0f, -1.0f, 0.0f}},
};

#define MAX_FACE_COUNT 1024
#define MAX_VERTEX_COUNT (6 * MAX_FACE_COUNT)
static vertex *vertex_list;
static int vertex_count;

static void add_face(int face, int x, int y, int block) {
    memcpy(vertex_list + vertex_count, cube_faces + 6 * face, 6 * sizeof(vertex));
    for (int i = 0; i < 6; i++) {
        vertex_list[vertex_count + i].position[0] += x;
        vertex_list[vertex_count + i].position[1] += x;
        vertex_list[vertex_count + i].texcoord[0] += (block - 1) * UV_X;
    }
    vertex_count += 6;
}

static DVLB_s *vshader_dvlb;
static shaderProgram_s program;
static int uLoc_projection, uLoc_modelView;
static int uLoc_lightVec, uLoc_lightHalfVec, uLoc_lightClr, uLoc_material;
static C3D_Mtx projection;
static C3D_Mtx material = {{
    {{0.0f, 0.2f, 0.2f, 0.2f}}, // Ambient
    {{0.0f, 0.4f, 0.4f, 0.4f}}, // Diffuse
    {{0.0f, 0.8f, 0.8f, 0.8f}}, // Specular
    {{1.0f, 0.0f, 0.0f, 0.0f}}, // Emission
}};

static C3D_Tex texture_tex;
static float angleX = 0.0, angleY = 0.0;

// Helper function for loading a texture from memory
static bool loadTextureFromMem(C3D_Tex *tex, C3D_TexCube *cube, const void *data, size_t size) {
    Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, false);
    if (!t3x)
        return false;

    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);
    return true;
}

static void sceneInit(void) {
    // Load the vertex shader, create a shader program and bind it
    vshader_dvlb = DVLB_ParseFile((u32 *)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    C3D_BindProgram(&program);

    // Get the location of the uniforms
    uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
    uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
    uLoc_lightVec     = shaderInstanceGetUniformLocation(program.vertexShader, "lightVec");
    uLoc_lightHalfVec = shaderInstanceGetUniformLocation(program.vertexShader, "lightHalfVec");
    uLoc_lightClr     = shaderInstanceGetUniformLocation(program.vertexShader, "lightClr");
    uLoc_material     = shaderInstanceGetUniformLocation(program.vertexShader, "material");

    // Configure attributes for use with the vertex shader
    C3D_AttrInfo *attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2=normal

    // Compute the projection matrix
    Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(80.0f), C3D_AspectRatioTop, 0.01f, 1000.0f, false);

    // Create the VBO (vertex buffer object)
    vertex_list  = linearAlloc(MAX_VERTEX_COUNT * sizeof(vertex));
    vertex_count = 36;
    // memcpy(vertex_list, cube_faces, vertex_count * sizeof(vertex));
    add_face(FACE_PX, 0, 0, 1);
    add_face(FACE_PY, 0, 0, 2);
    add_face(FACE_PZ, 0, 0, 3);
    add_face(FACE_MX, 0, 0, 4);
    add_face(FACE_MY, 0, 0, 5);
    add_face(FACE_MZ, 0, 0, 6);

    // Configure buffers
    C3D_BufInfo *bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vertex_list, sizeof(vertex), 3, 0x210);

    // Load the texture and bind it to the first texture unit
    if (!loadTextureFromMem(&texture_tex, NULL, texture_t3x, texture_t3x_size))
        svcBreak(USERBREAK_PANIC);
    C3D_TexSetFilter(&texture_tex, GPU_NEAREST, GPU_NEAREST);
    C3D_TexBind(0, &texture_tex);

    // Configure the first fragment shading substage to blend the texture color with
    // the vertex color (calculated by the vertex shader using a lighting algorithm)
    // See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
    C3D_TexEnv *env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
}

static void sceneRender(void) {
    // Calculate the modelView matrix
    C3D_Mtx modelView;
    Mtx_Identity(&modelView);
    Mtx_Translate(&modelView, 0.0, 0.0, -2.0, true);
    Mtx_RotateX(&modelView, angleX, true);
    Mtx_RotateY(&modelView, angleY, true);

    // Rotate the cube each frame
    angleX += M_PI / 180;
    angleY += M_PI / 360;

    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_material, &material);
    C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightVec, 0.0f, 0.0f, -1.0f, 0.0f);
    C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightHalfVec, 0.0f, 0.0f, -1.0f, 0.0f);
    C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightClr, 1.0f, 1.0f, 1.0f, 1.0f);

    // Draw the VBO
    C3D_DrawArrays(GPU_TRIANGLES, 0, vertex_count);
}

static void sceneExit(void) {
    // Free the texture
    C3D_TexDelete(&texture_tex);

    // Free the VBO
    linearFree(vertex_list);

    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}

int main() {
    // Initialize graphics
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // Initialize the render target
    C3D_RenderTarget *target = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

    // Initialize the scene
    sceneInit();

    // Main loop
    while (aptMainLoop()) {
        hidScanInput();

        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break; // break in order to return to hbmenu

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(target);
        sceneRender();
        C3D_FrameEnd(0);
    }

    // Deinitialize the scene
    sceneExit();

    // Deinitialize graphics
    C3D_Fini();
    gfxExit();
    return 0;
}
