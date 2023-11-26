#include "display.hpp"
#include "failure.hpp"
#include "matrix.hpp"
#include "vec3d.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

struct triangle {
  vec3d p[3];

  float illumination;
};

void printTriangle(triangle tri, bool pad = true) {
  std::cout << "illumination: " << tri.illumination << std::endl;
  for (int i = 0; i < 3; i++) {
    printVec3d(tri.p[i]);
  }
  if (pad)
    std::cout << "\n";
}

struct mesh {
  std::vector<triangle> tris;

  bool LoadFromObjectFile(std::string sFilename) {
    std::ifstream f(sFilename);
    if (!f.is_open()) {
      return false;
    }

    std::vector<vec3d> verts;

    while (!f.eof()) {
      char line[128];
      f.getline(line, 128);

      std::stringstream s;
      s << line;

      char junk;

      if (line[0] == 'v') {
        vec3d v;
        s >> junk >> v.x >> v.y >> v.z;
        verts.push_back(v);
      }
      if (line[0] == 'f') {
        int f[3];
        s >> junk >> f[0] >> f[1] >> f[2];
        tris.push_back({verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1]});
      }
    }

    return true;
  }
};

class mat4x4 {
public:
  float m[4][4];

  mat4x4() {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        m[i][j] = 0;
      }
    }
  }
};

void printMat4x4(mat4x4 m) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      std::cout << m.m[i][j] << " ";
    }
    std::cout << "\n";
  }
}

class olcEngine3D : public Display {
public:
  olcEngine3D() {}

private:
  mesh meshCube;
  mat4x4 matProj;

  vec3d vCamera;
  vec3d vLookDir;

  float fTheta = 0;
  float fYaw = 0;

  vec3d Matrix_MultiplyVector(mat4x4 &m, vec3d &i) {
    vec3d v;
    v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
    v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
    v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
    v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
    return v;
  }

  mat4x4 Matrix_PointAt(vec3d &pos, vec3d &target, vec3d &up) {
    vec3d newForward = Vector_Sub(target, pos);
    newForward = Vector_Normalise(newForward);

    vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
    vec3d newUp = Vector_Sub(up, a);
    newUp = Vector_Normalise(newUp);

    vec3d newRight = Vector_CrossProduct(newUp, newForward);

    mat4x4 matrix;
    matrix.m[0][0] = newRight.x;
    matrix.m[0][1] = newRight.y;
    matrix.m[0][2] = newRight.z;
    matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = newUp.x;
    matrix.m[1][1] = newUp.y;
    matrix.m[1][2] = newUp.z;
    matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = newForward.x;
    matrix.m[2][1] = newForward.y;
    matrix.m[2][2] = newForward.z;
    matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = pos.x;
    matrix.m[3][1] = pos.y;
    matrix.m[3][2] = pos.z;
    matrix.m[3][3] = 1.0f;
    return matrix;
  }

  mat4x4 Matrix_MakeIdentity() {
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
  }

  mat4x4 Matrix_MakeRotationX(float fAngleRad) {
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = cosf(fAngleRad);
    matrix.m[1][2] = sinf(fAngleRad);
    matrix.m[2][1] = -sinf(fAngleRad);
    matrix.m[2][2] = cosf(fAngleRad);
    matrix.m[3][3] = 1.0f;
    return matrix;
  }

  mat4x4 Matrix_MakeRotationY(float fAngleRad) {
    mat4x4 matrix;
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][2] = sinf(fAngleRad);
    matrix.m[2][0] = -sinf(fAngleRad);
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = cosf(fAngleRad);
    matrix.m[3][3] = 1.0f;
    return matrix;
  }

  mat4x4 Matrix_MakeRotationZ(float fAngleRad) {
    mat4x4 matrix;
    matrix.m[0][0] = cosf(fAngleRad);
    matrix.m[0][1] = sinf(fAngleRad);
    matrix.m[1][0] = -sinf(fAngleRad);
    matrix.m[1][1] = cosf(fAngleRad);
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    return matrix;
  }

  mat4x4 Matrix_MakeTranslation(float x, float y, float z) {
    mat4x4 matrix;
    matrix.m[0][0] = 1.0f;
    matrix.m[1][1] = 1.0f;
    matrix.m[2][2] = 1.0f;
    matrix.m[3][3] = 1.0f;
    matrix.m[3][0] = x;
    matrix.m[3][1] = y;
    matrix.m[3][2] = z;
    return matrix;
  }

  mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio,
                               float fNear, float fFar) {
    float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
    mat4x4 matrix;
    matrix.m[0][0] = fAspectRatio * fFovRad;
    matrix.m[1][1] = fFovRad;
    matrix.m[2][2] = fFar / (fFar - fNear);
    matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matrix.m[2][3] = 1.0f;
    matrix.m[3][3] = 0.0f;
    return matrix;
  }

  mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2) {
    mat4x4 matrix;
    for (int c = 0; c < 4; c++)
      for (int r = 0; r < 4; r++)
        matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] +
                         m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
    return matrix;
  }

  mat4x4
  Matrix_QuickInverse(mat4x4 &m) // Only for Rotation/Translation Matrices
  {
    mat4x4 matrix;
    matrix.m[0][0] = m.m[0][0];
    matrix.m[0][1] = m.m[1][0];
    matrix.m[0][2] = m.m[2][0];
    matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = m.m[0][1];
    matrix.m[1][1] = m.m[1][1];
    matrix.m[1][2] = m.m[2][1];
    matrix.m[1][3] = 0.0f;
    matrix.m[2][0] = m.m[0][2];
    matrix.m[2][1] = m.m[1][2];
    matrix.m[2][2] = m.m[2][2];
    matrix.m[2][3] = 0.0f;
    matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] +
                       m.m[3][2] * matrix.m[2][0]);
    matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] +
                       m.m[3][2] * matrix.m[2][1]);
    matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] +
                       m.m[3][2] * matrix.m[2][2]);
    matrix.m[3][3] = 1.0f;
    return matrix;
  }

  vec3d Vector_Add(vec3d &v1, vec3d &v2) {
    return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
  }

  vec3d Vector_Sub(vec3d &v1, vec3d &v2) {
    return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
  }

  vec3d Vector_Mul(vec3d &v1, float k) {
    return {v1.x * k, v1.y * k, v1.z * k};
  }

  vec3d Vector_Div(vec3d &v1, float k) {
    return {v1.x / k, v1.y / k, v1.z / k};
  }

  float Vector_DotProduct(vec3d &v1, vec3d &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  float Vector_Length(vec3d &v) { return sqrtf(Vector_DotProduct(v, v)); }

  vec3d Vector_Normalise(vec3d &v) {
    float l = Vector_Length(v);
    return {v.x / l, v.y / l, v.z / l};
  }

  vec3d Vector_CrossProduct(vec3d &v1, vec3d &v2) {
    vec3d v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
  }

  vec3d Vector_IntersectPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart,
                              vec3d &lineEnd) {
    plane_n = Vector_Normalise(plane_n);
    float plane_d = -Vector_DotProduct(plane_n, plane_p);
    float ad = Vector_DotProduct(lineStart, plane_n);
    float bd = Vector_DotProduct(lineEnd, plane_n);
    float t = (-plane_d - ad) / (bd - ad);
    vec3d lineStartToEnd = Vector_Sub(lineEnd, lineStart);
    vec3d lineToIntersect = Vector_Mul(lineStartToEnd, t);
    return Vector_Add(lineStart, lineToIntersect);
  }

public:
  bool OnUserCreate() {
    if (!meshCube.LoadFromObjectFile("res/axis.obj")) {
      fail("Could not find file");
    }

    matProj = Matrix_MakeProjection(
        90.0f, (float)this->height / (float)this->width, 0.1f, 1000.0f);

    return true;
  }

  bool OnUserUpdate(float fElapsedTime, Keyboard *keyboard) {
    if (keyboard->ARROW_UP)
      vCamera.y -= 8.0f * fElapsedTime;
    if (keyboard->ARROW_DOWN)
      vCamera.y += 8.0f * fElapsedTime;
    if (keyboard->ARROW_LEFT)
      vCamera.x -= 8.0f * fElapsedTime;
    if (keyboard->ARROW_RIGHT)
      vCamera.x += 8.0f * fElapsedTime;

    vec3d vForward = Vector_Mul(vLookDir, 8.0f * fElapsedTime);

    if (keyboard->W)
      vCamera = Vector_Add(vCamera, vForward);
    if (keyboard->S)
      vCamera = Vector_Sub(vCamera, vForward);
    if (keyboard->A)
      fYaw -= 2.0f * fElapsedTime;
    if (keyboard->D)
      fYaw += 2.0f * fElapsedTime;

    this->clear();

    mat4x4 matRotZ, matRotX;
    // fTheta += 1.0f * fElapsedTime;

    matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
    matRotX = Matrix_MakeRotationX(fTheta);

    mat4x4 matTrans;
    matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 16.0f);

    mat4x4 matWorld;
    matWorld = Matrix_MakeIdentity();
    matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
    matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

    vec3d vUp = {0, 1, 0};
    vec3d vTarget = {0, 0, 1};
    mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
    vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);

    mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

    mat4x4 matView = Matrix_QuickInverse(matCamera);

    std::vector<triangle> vecTrianglesToRaster;

    for (auto tri : meshCube.tris) {
      triangle triProjected, triTransformed, triViewed;

      triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
      triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
      triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

      vec3d normal, line1, line2;

      line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
      line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

      normal = Vector_CrossProduct(line1, line2);

      normal = Vector_Normalise(normal);

      vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

      if (Vector_DotProduct(normal, vCameraRay) < 0.0f) {

        vec3d light_direction = {0.0f, 0.0f, -1.0f};
        light_direction = Vector_Normalise(light_direction);

        float dp = std::max(0.1f, Vector_DotProduct(light_direction, normal));

        triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
        triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
        triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

        triProjected.p[0] = Matrix_MultiplyVector(matProj, triViewed.p[0]);
        triProjected.p[1] = Matrix_MultiplyVector(matProj, triViewed.p[1]);
        triProjected.p[2] = Matrix_MultiplyVector(matProj, triViewed.p[2]);

        triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
        triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
        triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

        vec3d vOffsetView = {1, 1, 0};
        triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
        triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
        triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);
        triProjected.p[0].x *= 0.5f * (float)this->width;
        triProjected.p[0].y *= 0.5f * (float)this->height;
        triProjected.p[1].x *= 0.5f * (float)this->width;
        triProjected.p[1].y *= 0.5f * (float)this->height;
        triProjected.p[2].x *= 0.5f * (float)this->width;
        triProjected.p[2].y *= 0.5f * (float)this->height;

        triProjected.illumination = dp;
        vecTrianglesToRaster.push_back(triProjected);
      }
    }

    std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(),
              [](triangle &t1, triangle &t2) {
                float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
                float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
                return z1 > z2;
              });

    for (auto &triProjected : vecTrianglesToRaster) {
      this->fillTriangle(
          triProjected.p[0], triProjected.p[1], triProjected.p[2],
          (static_cast<Uint32>(0xff * triProjected.illumination) << 24) |
              (static_cast<Uint32>(0xff * triProjected.illumination) << 16) |
              (static_cast<Uint32>(0xff * triProjected.illumination) << 8) |
              0xff);
    }

    return true;
  }
};

int main() {
  olcEngine3D demo = olcEngine3D();

  Keyboard *keyboard = initKeyboard();

  demo.OnUserCreate();
  while (true) {
    demo.poll(keyboard);
    demo.OnUserUpdate(1.0f / 60.0f, keyboard);
    demo.draw();

    SDL_Delay(1000.0f / 60.0f);
  }

  return 0;
}
