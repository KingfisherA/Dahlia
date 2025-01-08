#include "GraphicsEngine/Maths/Geometry.h"
#include "GraphicsEngine/Utilities/Assert.h"
#include "GraphicsEngine/Maths/Maths.h"

#include <numbers>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <string>
#include <unordered_map>
#include <cstring>
#include <valarray>


namespace GEng {
    //
    GENG_API struct hash_pair {
        template<class T1, class T2>
        size_t operator()(const std::pair<T1, T2> &p) const {
            auto hash1 = std::hash<T1>{}(p.first);
            auto hash2 = std::hash<T2>{}(p.second);
            return hash1 ^ hash2;
        }
    };

    //
    //
#pragma region Geometry
    //
    glm::vec3 Geometry::CalculateNormalCounterclockwise(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2) {
        return glm::normalize(glm::cross(v1 - v0, v2 - v0));
    }

    //
    glm::vec3 Geometry::CalculateNormalClockwise(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2) {
        return glm::normalize(glm::cross(v1 - v0, v1 - v2));
    }

    //
    void Geometry::CalculateNormals(const std::vector<float> &vertices, const std::vector<uint32_t> &triangles,
                                    std::vector<float> &normals) {
        normals.resize(vertices.size());

        for (size_t i = 0; i < triangles.size() / 3; i++) {
            TriangleI tri = {triangles[i * 3 + 0], triangles[i * 3 + 1], triangles[i * 3 + 2]};

            glm::vec3 normal = CalculateNormalClockwise(
                {vertices[tri.x * 3 + 0], vertices[tri.x * 3 + 1], vertices[tri.x * 3 + 2]},
                {vertices[tri.y * 3 + 0], vertices[tri.y * 3 + 1], vertices[tri.y * 3 + 2]},
                {vertices[tri.z * 3 + 0], vertices[tri.z * 3 + 1], vertices[tri.z * 3 + 2]});

            normals[tri.x * 3 + 0] = normal.x;
            normals[tri.x * 3 + 1] = normal.y;
            normals[tri.x * 3 + 2] = normal.z;
        }
    }

    //
    Mesh_F32 Geometry::CreateUVSphere(float radius, uint32_t lattitudes, uint32_t longitudes) {
        GENG_ASSERT(lattitudes < 3, Geometry::CreateSphere, "The minimum number of slices is 3");
        GENG_ASSERT(longitudes < 3, Geometry::CreateSphere, "The minimum number of stacks is 3");

        size_t numberOfVerts = ((size_t) lattitudes) * (longitudes - 1) + 2;
        size_t numberOfTriangles = ((size_t) longitudes - 2) * lattitudes * 2 + 2 * lattitudes;

        Mesh_F32 sphereMesh(3, numberOfVerts * 3, numberOfVerts * 3,
                            numberOfVerts * 2, numberOfTriangles * 3);

        std::vector<float> &vertices = sphereMesh.GetVertexArray();
        std::vector<float> &normals = sphereMesh.GetNormalsArray();
        std::vector<float> &texCoord = sphereMesh.GetTexCoordsArray();
        std::vector<uint32_t> &triangles = sphereMesh.GetIndicesArray();

        size_t i = 0, j = 0, idx = 0;

        // Write data for top most vertex
        vertices[0] = 0.0f;
        vertices[1] = 1.0f;
        vertices[2] = 0.0f;

        texCoord[0] = 0.5f;
        texCoord[1] = 0.0f;

        for (i = 1; i < longitudes; i++) {
            // Compute the V texture coordinate
            float V = float(i) / float(longitudes);
            // Compute the stack's angle
            float Phi = V * Maths::PI;

            for (j = 0; j < lattitudes; j++) {
                // Compute the U texture coordinate
                float U = float(j) / float(lattitudes);
                // Compute the slice's angle
                float Theta = U * GEng::Maths::Tau;

                idx = i * 3 + j * lattitudes * 3;

                glm::vec3 currentVertex = {cos(Theta) * sin(Phi), cos(Phi), sin(Theta) * sin(Phi)};

                vertices[idx + 0] = currentVertex.x;
                vertices[idx + 1] = currentVertex.y;
                vertices[idx + 2] = currentVertex.z;

                texCoord[idx + 0] = ((atan2(currentVertex.x, currentVertex.z) + (float) Maths::PI) / (float) Maths::PI)
                                    * 0.5f;
                texCoord[idx + 1] = ((acos(currentVertex.y) + (float) Maths::PI) / (float) Maths::PI) - 1.0f;
            }
        }

        // Write data for bottom most vertex
        idx = i * 3 + j * lattitudes * 3;
        vertices[idx + 0] = 0.0f;
        vertices[idx + 1] = -1.0f;
        vertices[idx + 2] = 0.0f;
        texCoord[idx + 0] = 0.5f;
        texCoord[idx + 1] = 1.0f;

        const size_t topTriangleCount = lattitudes;
        const size_t mainFaceCount = (longitudes - 2) * lattitudes;

        idx = 0;

        for (i = 0; i < topTriangleCount; i++) {
            triangles[idx++] = i == topTriangleCount - 1 ? 1 : i + 2;
            triangles[idx++] = i + 1;
            triangles[idx++] = 0;
        }

        // Face count instead of triangle count as two triangles are written per iteration
        for (i = topTriangleCount; i < mainFaceCount + topTriangleCount; i++) {
            if ((i + 1) % (lattitudes) == 0) {
                // First triangle's indices
                triangles[idx++] = i - lattitudes + 2;
                triangles[idx++] = i + 1;
                triangles[idx++] = i - lattitudes + 1;
                // Second triangle's indices
                triangles[idx++] = i - lattitudes + 2;
                triangles[idx++] = i - lattitudes + 1;
                triangles[idx++] = i - lattitudes - (lattitudes - 1) + 1;

                continue;
            }

            // First triangle's indices
            triangles[idx++] = i + 2;
            triangles[idx++] = i + 1;
            triangles[idx++] = i - lattitudes + 1;
            // Second triangle's indices
            triangles[idx++] = i - lattitudes + 1;
            triangles[idx++] = i - lattitudes + 2;
            triangles[idx++] = i + 2;
        }

        for (i = 0; i < topTriangleCount; i++) {
            size_t triIndex = i + vertices.size() - 1 - lattitudes;

            triangles[idx++] = i == topTriangleCount - 1 ? triIndex - (lattitudes - 1) : triIndex + 1;
            triangles[idx++] = vertices.size() - 1;
            triangles[idx++] = triIndex;
        }

        std::vector<uint32_t> wrappedCoords = DetectSphereWrappedUVCoords(triangles, texCoord, longitudes);
        FixSphereWrappedUV(wrappedCoords, vertices, triangles, texCoord);

        normals = vertices;

        return sphereMesh;
    }

    //
    Mesh_F32 Geometry::CreateQuadsphere(float radius, uint32_t subdivisions) {
        size_t noOfVerts = subdivisions * subdivisions * 6;
        size_t noOfTriangles = (subdivisions - 1) * (subdivisions - 1) * 2 * 6;

        Mesh_F32 sphereMesh(3, noOfVerts * 3, noOfVerts * 3, noOfVerts * 2,
                            noOfTriangles * 3);

        std::vector<float> &vertices = sphereMesh.GetVertexArray();
        std::vector<float> &normals = sphereMesh.GetNormalsArray();
        std::vector<float> &texCoords = sphereMesh.GetTexCoordsArray();
        std::vector<uint32_t> &triangles = sphereMesh.GetIndicesArray();

        constexpr glm::vec3 localUps[] = {
            glm::vec3{0.0f, 1.0f, 0.0f},
            glm::vec3{0.0f, -1.0f, 0.0f},

            glm::vec3{1.0f, 0.0f, 0.0f},
            glm::vec3{-1.0f, 0.0f, 0.0f},

            glm::vec3{0.0f, 0.0f, 1.0f},
            glm::vec3{0.0f, 0.0f, -1.0f}
        };

        glm::vec3 axisA;
        glm::vec3 axisB;

        size_t triIndex = 0;

        for (uint32_t faces = 0; faces < 6; faces++) {
            axisA.x = localUps[faces].y;
            axisA.y = localUps[faces].z;
            axisA.z = localUps[faces].x;

            axisB = glm::cross(localUps[faces], axisA);

            for (uint32_t y = 0; y < subdivisions; y++) {
                for (uint32_t x = 0; x < subdivisions; x++) {
                    glm::vec2 percent(x, y);
                    percent /= (float) (subdivisions - 1);

                    size_t index = (x + y * subdivisions + faces * subdivisions * subdivisions);
                    glm::vec3 vertex = glm::normalize(
                        localUps[faces] +
                        (percent.x - 0.5f) * 2.0f * axisA +
                        (percent.y - 0.5f) * 2.0f * axisB);
                    vertices[index * 3 + 0] = vertex.x;
                    vertices[index * 3 + 1] = vertex.y;
                    vertices[index * 3 + 2] = vertex.z;

                    if (x != subdivisions - 1 && y != subdivisions - 1) {
                        // trianglesArray[triIndex] = index;
                        // trianglesArray[triIndex + 1] = index + subdivisions + 1;
                        // trianglesArray[triIndex + 2] = index + subdivisions;

                        // trianglesArray[triIndex + 3] = index;
                        // trianglesArray[triIndex + 4] = index + 1;
                        // trianglesArray[triIndex + 5] = index + subdivisions + 1;

                        triangles[triIndex * 3 + 0] = index;
                        triangles[triIndex * 3 + 1] = index + subdivisions + 1;
                        triangles[triIndex * 3 + 2] = index + subdivisions;

                        triangles[(triIndex + 1) * 3 + 0] = index;
                        triangles[(triIndex + 1) * 3 + 1] = index + 1;
                        triangles[(triIndex + 1) * 3 + 2] = index + subdivisions + 1;

                        triIndex += 2;
                    }
                }
            }
        }

        for (size_t i = 0; i < vertices.size() / 3; i++) {
            const glm::vec3 normal = {vertices[i * 3 + 0], vertices[i * 3 + 1], vertices[i * 3 + 2]};

            texCoords[i * 2 + 0] = ((atan2(normal.x, normal.z) + Maths::PI) / Maths::PI) * 0.5f;
            texCoords[i * 2 + 1] = ((acos(normal.y) + Maths::PI) / Maths::PI) - 1.0;
        }

        normals = vertices;

        for (float &v: vertices)
            v *= radius;

        return sphereMesh;
    }

    //
    Mesh_F32 Geometry::CreateIcosahedron(float size) {
        Mesh_F32 icosahedronMesh(3,
                                 ICOSAHEDRON_VERTS.size(),
                                 ICOSAHEDRON_VERTS.size(),
                                 0,
                                 ICOSAHEDRON_TRIS.size());

        size_t i = 0;
        for (; i < ICOSAHEDRON_VERTS.size(); i++) {
            icosahedronMesh.GetNormalsArray()[i] = (float) ICOSAHEDRON_VERTS[i];
            icosahedronMesh.GetVertexArray()[i] = (float) ICOSAHEDRON_VERTS[i] * size;
        }
        for (i = 0; i < ICOSAHEDRON_TRIS.size(); i++) {
            icosahedronMesh.GetIndicesArray()[i] = ICOSAHEDRON_TRIS[i];
        }

        return icosahedronMesh;
    }

    //
    // //
    Mesh_F32 Geometry::CreateIcosphere(float radius, uint32_t subdivisions) {
        auto PrecomputeNoTriangles = [](uint32_t subdivisions) -> uint32_t {
            return 20 * Maths::IntegerPow<uint32_t>(4, subdivisions);
        };

        auto PrecomputeVertexAmount = [](uint32_t subdivisions) -> uint32_t {
            uint32_t result = 12;

            for (uint32_t i = 1; i <= subdivisions; i++) {
                result += (30 * Maths::IntegerPow((uint32_t) 4, i - 1));
            }
            return result;
        };

        std::vector<Vertex> icoVertices;
        icoVertices.reserve(PrecomputeVertexAmount(subdivisions));
        std::vector<TriangleI> icoTriangles;
        icoTriangles.reserve(PrecomputeNoTriangles(subdivisions));
        std::vector<TriangleI> icoTrianglesBuffer;
        icoTrianglesBuffer.reserve(subdivisions > 0 ? PrecomputeNoTriangles(subdivisions - 1) : 0);

        uint32_t i = 0;

        // Write Icosahedron Data
        for (i = 0; i < ICOSAHEDRON_VERTS.size() / 3; i++) {
            icoVertices.emplace_back(
                ICOSAHEDRON_VERTS[i * 3 + 0],
                ICOSAHEDRON_VERTS[i * 3 + 1],
                ICOSAHEDRON_VERTS[i * 3 + 2]);
        }
        for (i = 0; i < ICOSAHEDRON_TRIS.size() / 3; i++) {
            icoTriangles.emplace_back(
                ICOSAHEDRON_TRIS[i * 3 + 0],
                ICOSAHEDRON_TRIS[i * 3 + 1],
                ICOSAHEDRON_TRIS[i * 3 + 2]);
        }

        std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, hash_pair> lookupTable;

        // Lamda functions for subdividing icosahedron
        auto CalculateVertex = [&lookupTable, &icoVertices](uint32_t first, uint32_t second) -> uint32_t {
            std::pair<uint32_t, uint32_t> key(first, second);
            if (key.first > key.second) {
                std::swap(key.first, key.second);
            }

            auto inserted = lookupTable.insert({key, icoVertices.size()});
            if (inserted.second) {
                const glm::vec3 &edge0 = icoVertices[first];
                const glm::vec3 &edge1 = icoVertices[second];
                icoVertices.push_back(glm::normalize(edge0 + edge1));
            }

            return inserted.first->second;
        };
        auto Subdivide = [&CalculateVertex, &icoTriangles, &icoTrianglesBuffer, &icoVertices, &lookupTable]() -> void {
            TriangleI midPointTri;
            lookupTable.clear();
            icoTriangles.clear();

            for (TriangleI &currentTri: icoTrianglesBuffer) {
                for (int edge = 0; edge < 3; ++edge) {
                    midPointTri[edge] = CalculateVertex(currentTri[edge], currentTri[(edge + 1) % 3]);
                }

                icoTriangles.emplace_back(currentTri[0], midPointTri[0], midPointTri[2]);
                icoTriangles.emplace_back(currentTri[1], midPointTri[1], midPointTri[0]);
                icoTriangles.emplace_back(currentTri[2], midPointTri[2], midPointTri[1]);
                icoTriangles.emplace_back(midPointTri[0], midPointTri[1], midPointTri[2]);
            }
        };

        if (subdivisions > 0) {
            icoTrianglesBuffer = icoTriangles;
        }

        // Perform subdivisions
        for (i = 0; i < subdivisions; ++i) {
            Subdivide();

            if (i != subdivisions - 1) {
                icoTrianglesBuffer = icoTriangles;
            }
        }

        icoTrianglesBuffer.clear();

        Mesh_F32 icoSphereMesh(3, icoVertices.size() * 3, icoVertices.size() * 3, icoVertices.size() * 2,
                               icoTriangles.size() * 3);

        // Populate texture coordinate array
        for (i = 0; i < icoVertices.size(); i++) {
            const glm::vec3 &normal = icoVertices[i];

            icoSphereMesh.GetTexCoordsArray()[i * 2 + 0] = ((atan2(normal.x, normal.z) + Maths::PI) / Maths::PI) * 0.5f;
            icoSphereMesh.GetTexCoordsArray()[i * 2 + 1] = ((acos(normal.y) + Maths::PI) / Maths::PI) - 1.0;

            icoSphereMesh.GetVertexArray()[i * 3 + 0] = icoVertices[i][0] * radius;
            icoSphereMesh.GetVertexArray()[i * 3 + 1] = icoVertices[i][1] * radius;
            icoSphereMesh.GetVertexArray()[i * 3 + 2] = icoVertices[i][2] * radius;

            icoSphereMesh.GetNormalsArray()[i * 3 + 0] = icoVertices[i][0];
            icoSphereMesh.GetNormalsArray()[i * 3 + 1] = icoVertices[i][1];
            icoSphereMesh.GetNormalsArray()[i * 3 + 2] = icoVertices[i][2];

            icoSphereMesh.GetIndicesArray()[i * 3 + 0] = icoTriangles[i][0];
            icoSphereMesh.GetIndicesArray()[i * 3 + 1] = icoTriangles[i][1];
            icoSphereMesh.GetIndicesArray()[i * 3 + 2] = icoTriangles[i][2];
        }

        // Fix texture seam issue by duplicating verticies and correcting texture coordinates
        std::vector<uint32_t> wrappedCoords = DetectSphereWrappedUVCoords(
            icoSphereMesh.GetIndicesArray(), icoSphereMesh.GetTexCoordsArray());
        FixSphereWrappedUV(wrappedCoords, icoSphereMesh.GetVertexArray(), icoSphereMesh.GetIndicesArray(),
                           icoSphereMesh.GetTexCoordsArray());

        // Return icosphere mesh
        return icoSphereMesh;
    }

    //
    Mesh_F32 Geometry::CreateCylinder(float radius, float height, size_t segments) {
        size_t noOfVertices = 2 * segments + 2;
        size_t noTriangles = 4 * segments;

        Mesh_F32 cylinderMesh(3, noOfVertices * 3, 0, 0, noTriangles * 3);

        std::vector<float> &vertices = cylinderMesh.GetVertexArray();
        std::vector<uint32_t> &indices = cylinderMesh.GetIndicesArray();

        size_t idx = 0, i = 0;

        vertices[0] = 0.0f;
        vertices[1] = 0.0f;
        vertices[2] = height * 0.5f;

        for (i = 1; i < segments + 1; i++) {
            float theta = 2.0f * std::numbers::pi * float(i) / float(segments);
            glm::vec3 v1(radius * cos(theta), radius * sin(theta), height * 0.5f);
            glm::vec3 v2(v1.x, v1.y, -height * 0.5f);
            vertices[i * 3 + 0] = v1.x;
            vertices[i * 3 + 1] = v1.y;
            vertices[i * 3 + 2] = v1.z;
            // normals[i + 1] = glm::vec3(v1.x, v2.y, 0.0f);
            vertices[(i + segments) * 3 + 0] = v2.x;
            vertices[(i + segments) * 3 + 1] = v2.y;
            vertices[(i + segments) * 3 + 2] = v2.z;
            // normals[i + 1 + segments] = glm::vec3(v1.x, v2.y, 0.0f);
        }

        vertices[(2 * segments + 1) * 3 + 0] = 0.0f; //{0.0f, 0.0f, -height * 0.5f};
        vertices[(2 * segments + 1) * 3 + 1] = 0.0f;
        vertices[(2 * segments + 1) * 3 + 2] = -height * 0.5f;


        for (i = 0; i < segments; i++) {
            indices[idx++] = (i + 1);
            indices[idx++] = (0);
            indices[idx++] = (i == segments - 1 ? 1 : i + 2);
        }

        for (i = 0; i < segments; i++) {
            indices[idx++] = (i == segments - 1 ? 1 : i + 2);
            indices[idx++] = (i + 1);
            indices[idx++] = (segments + i + 1);

            indices[idx++] = (segments + i + 1);
            indices[idx++] = (i == segments - 1 ? segments + 1 : segments + i + 2);
            indices[idx++] = (i == segments - 1 ? 1 : i + 2);
        }

        for (i = 0; i < segments; i++) {
            indices[idx++] = (i + segments + 1);
            indices[idx++] = (2 * segments + 1);
            indices[idx++] = (i == segments - 1 ? segments + 1 : i + segments + 2);
        }

        return cylinderMesh;
    }

    //
    Mesh_F32 Geometry::CreateCone(float height, float radius, size_t segments) {
        segments = std::max<size_t>(segments, 3);

        size_t noOfVertices = segments + 2;
        size_t noOfTriangles = segments * 2;
        Mesh_F32 coneMesh(3, noOfVertices * 3, noOfVertices * 3,
                          0, noOfTriangles * 3);


        std::vector<float> &vertices = coneMesh.GetVertexArray();
        std::vector<float> &normals = coneMesh.GetNormalsArray();
        std::vector<uint32_t> &indices = coneMesh.GetIndicesArray();

        //Vertex 0
        vertices[0] = 0.0f;
        vertices[1] = 0.0f;
        vertices[2] = height;

        //Vertex 1
        vertices[3] = 0.0f;
        vertices[4] = 0.0f;
        vertices[5] = 0.0f;

        size_t idx = 6;
        for (size_t i = 0; i < segments; i++) {
            float theta = 2.0f * std::numbers::pi * float(i) / float(segments);


            vertices[idx++] = radius * cos(theta);
            vertices[idx++] = radius * sin(theta);
            vertices[idx++] = 0.0f;
        }

        indices[0] = 1;
        indices[1] = 2;
        indices[2] = segments + 1;

        indices[3] = 0;
        indices[4] = segments + 1;
        indices[5] = 2;

        idx = 6;
        for (size_t i = 2; i < segments + 1; i++) {
            indices[idx++] = 1;
            indices[idx++] = i + 1;
            indices[idx++] = i;

            indices[idx++] = 0;
            indices[idx++] = i;
            indices[idx++] = i + 1;
        }

        CalculateNormals(vertices, indices, normals);

        return coneMesh;
    }

    //
    Mesh_F32 Geometry::CreatePyramid(float size) {
        return CreateCone(1.0f, 1.0f, 4);
    }

    //
    Mesh_F32 Geometry::CreateOctahedron(float height, float size) {
        Mesh_F32 octahedronMesh(3, 0, 0, 0, 0);

        octahedronMesh.GetVertexArray() = {
            0.0f, 0.0f, height,
            -size, -size, 0.0f,
            size, -size, 0.0f,
            size, size, 0.0f,
            -size, size, 0.0f,
            0.0f, 0.0f, -height
        };
        octahedronMesh.GetNormalsArray() = {
            0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            0.0f, 0.0f, -1.0f
        };
        octahedronMesh.GetIndicesArray() = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 4,
            0, 4, 1,
            2, 1, 5,
            3, 2, 5,
            4, 3, 5,
            4, 1, 5
        };

        return octahedronMesh;
    }

    //
    Mesh_F32 Geometry::CreateTetrahedron(float size) {
        return CreateCone(1.0f, 1.0f, 3);
    }

    //
    Mesh_F32 Geometry::CreateDodecahedron(float size) {
        Mesh_F32 dodecahedronMesh(3, 0, 0, 0, 0);

        dodecahedronMesh.GetVertexArray() = {
            0.577350, 0.577350, -0.57735,
            0.577350, -0.577350, -0.57735,
            0.577350, 0.577350, 0.57735,
            0.577350, -0.577350, 0.57735,
            -0.577350, 0.577350, -0.57735,
            -0.577350, -0.577350, -0.57735,
            -0.577350, 0.577350, 0.57735,
            -0.577350, -0.577350, 0.57735,
            0.356822, 0.000000, -0.93417,
            -0.356822, 0.000000, -0.93417,
            0.356822, 0.000000, 0.93417,
            -0.356822, 0.000000, 0.93417,
            0.934172, 0.356822, 0.00000,
            0.934172, -0.356822, 0.00000,
            -0.934172, 0.356822, 0.00000,
            -0.934172, -0.356822, 0.00000,
            0.000000, 0.934172, -0.35682,
            0.000000, 0.934172, 0.35682,
            0.000000, -0.934172, -0.35682,
            0.000000, -0.934172, 0.356822
        };

        dodecahedronMesh.GetNormalsArray() = dodecahedronMesh.GetVertexArray();

        dodecahedronMesh.GetIndicesArray() = {
            8, 4, 16,
            12, 1, 8,
            16, 2, 12,
            1, 5, 9,
            2, 3, 13,
            4, 6, 17,
            5, 14, 4,
            11, 2, 17,
            19, 1, 13,
            15, 18, 19,
            11, 14, 15,
            19, 10, 11,
            16, 0, 8,
            8, 9, 4,
            8, 0, 12,
            12, 13, 1,
            12, 0, 16,
            16, 17, 2,
            9, 8, 1,
            1, 18, 5,
            13, 12, 2,
            2, 10, 3,
            17, 16, 4,
            4, 14, 6,
            4, 9, 5,
            5, 15, 14,
            17, 6, 11,
            11, 10, 2,
            13, 3, 19,
            19, 18, 1,
            19, 7, 15,
            15, 5, 18,
            15, 7, 11,
            11, 6, 14,
            11, 7, 19,
            19, 3, 10
        };

        return dodecahedronMesh;
    }

    //
    Mesh_F32 Geometry::CreateQuad() {
        constexpr float quadVerticies[] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
        constexpr uint32_t quadIndicies[] = {0, 1, 3, 3, 1, 2};

        constexpr uint32_t vertexCount = sizeof(quadVerticies) / sizeof(float);
        constexpr uint32_t indiciesCount = sizeof(quadIndicies) / sizeof(uint32_t);

        Mesh_F32 quadMesh(2, vertexCount, 0, 0, indiciesCount);
        std::memcpy(quadMesh.GetVertexArray().data(), &quadVerticies, vertexCount * sizeof(float));
        std::memcpy(quadMesh.GetIndicesArray().data(), &quadIndicies, indiciesCount * sizeof(uint32_t));

        return quadMesh;
    }

    //
    Mesh_F32 Geometry::CreateSquircle(float radius, uint32_t roundness, uint32_t segments) {
        size_t noOfVerts = segments * 2 + 1;
        size_t noOfTriangles = (segments - 1) * 2;
        Mesh_F32 squircleMesh(2, noOfVerts * 2, 0, 0, noOfTriangles * 3);


        std::vector<float> &vertices = squircleMesh.GetVertexArray();
        std::vector<uint32_t> &indices = squircleMesh.GetIndicesArray();

        float n = (float) roundness * 2.0f;
        auto squircleEq = [](float x, float n) {
            return pow(1.0f - pow(x, n), 1.0f / n);
        };
        float halfX = squircleEq(0.5f, n);

        vertices[0] = 0.0f;
        vertices[1] = 0.0f;


        size_t idx = 0;
        for (uint32_t i = 0; i < segments; i++) {
            float x = (glm::smoothstep(0.0f, 1.0f, float(i) / (float) (segments - 1)) - 0.5f) * 2.0f;
            float y = squircleEq(x, n);

            idx = (i + 1) * 2;
            vertices[idx + 0] = x;
            vertices[idx + 1] = y;

            idx = (i + segments + 1) * 2;
            vertices[idx + 0] = x;
            vertices[idx + 1] = -y;
        }

        idx = 0;

        for (uint32_t i = 0; i < noOfTriangles / 2; i++) {
            indices[idx++] = 0;
            indices[idx++] = i + 1;
            indices[idx++] = i + 2;

            indices[idx++] = 0;
            indices[idx++] = i + 1 + segments;
            indices[idx++] = i + 2 + segments;
        }


        return squircleMesh;
    }

    Mesh_F32 Geometry::CreateZSurface(const glm::vec3 scaling, const glm::vec2 axisStart, const glm::vec2 axisEnd,
                                      uint32_t resolution, const std::function<float(float, float)> &func) {
        Mesh_F32 surfaceMesh = CreateGrid(1.0f, resolution, resolution);
        std::vector<float> &vertices = surfaceMesh.GetVertexArray();

        for (size_t i = 0; i <= resolution; i++)
            for (size_t j = 0; j <= resolution; j++) {
                size_t index = (i * (resolution + 1) + j) * 3;
                float &x = vertices[index + 0];
                float &y = vertices[index + 1];
                float &z = vertices[index + 2];

                float funcX = Maths::Remap(x, 0.0f, 1.0f, axisStart.x, axisEnd.x);
                float funcZ = Maths::Remap(z, 0.0f, 1.0f, axisStart.y, axisEnd.y);

                y = -func(funcX, funcZ);

                x *= scaling.x;
                z *= scaling.y;
                y *= scaling.z;
            }


        return surfaceMesh;
    }

    //
    Mesh_F32 Geometry::CreateGrid(float scale, uint32_t slices, uint32_t stacks) {
        uint32_t vertexCount = (slices + 1) * (stacks + 1);
        uint32_t noOfTriangles = (slices + 1) * (stacks + 1) * 2;

        Mesh_F32 gridMesh(3, vertexCount * 3, 0, 0, noOfTriangles * 3);
        std::vector<float> &vertices = gridMesh.GetVertexArray();
        std::vector<uint32_t> &indices = gridMesh.GetIndicesArray();

        uint32_t i = 0, j = 0, idx = 0;

        for (j = 0; j <= stacks; ++j) {
            for (i = 0; i <= slices; ++i) {
                float x = ((float) i / (float) slices) * scale;
                float y = 0;
                float z = ((float) j / (float) stacks) * scale;

                //idx = (j + i * slices);
                vertices[idx++] = x;
                vertices[idx++] = y;
                vertices[idx++] = z;
            }
        }

        idx = 0;

        for (j = 0; j < stacks; j++)
            for (i = 0; i < slices; i++) {
                uint32_t idx2 = j * (slices + 1) + i;
                indices[idx++] = idx2;
                indices[idx++] = idx2 + slices + 2;
                indices[idx++] = idx2 + 1;


                indices[idx++] = idx2;
                indices[idx++] = idx2 + slices + 1;
                indices[idx++] = idx2 + slices + 2;
            }

        return gridMesh;
    }

    //
    // //
    Mesh_F32 Geometry::CreateCube(float size) {
        constexpr glm::vec3 cubeVerticies[] = {
            // Front
            {-1, -1, 1},
            {1, -1, 1},
            {1, 1, 1},
            {1, 1, 1},
            {-1, 1, 1},
            {-1, -1, 1},

            // Right
            {1, -1, 1},
            {1, -1, -1},
            {1, 1, -1},
            {1, 1, -1},
            {1, 1, 1},
            {1, -1, 1},

            // Back
            {-1, 1, -1},
            {1, 1, -1},
            {1, -1, -1},
            {1, -1, -1},
            {-1, -1, -1},
            {-1, 1, -1},

            // Left
            {-1, -1, -1},
            {-1, -1, 1},
            {-1, 1, 1},
            {-1, 1, 1},
            {-1, 1, -1},
            {-1, -1, -1},

            // Bottom
            {-1, -1, -1},
            {1, -1, -1},
            {1, -1, 1},
            {1, -1, 1},
            {-1, -1, 1},
            {-1, -1, -1},

            // Top
            {-1, 1, 1},
            {1, 1, 1},
            {1, 1, -1},
            {1, 1, -1},
            {-1, 1, -1},
            {-1, 1, 1}
        };
        constexpr glm::vec3 cubeNormals[] = {
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},

            {1, 0, 0},
            {1, 0, 0},
            {1, 0, 0},
            {1, 0, 0},
            {1, 0, 0},
            {1, 0, 0},

            {0, 0, -1},
            {0, 0, -1},
            {0, 0, -1},
            {0, 0, -1},
            {0, 0, -1},
            {0, 0, -1},

            {-1, 0, 0},
            {-1, 0, -0},
            {-1, 0, 0},
            {-1, 0, -0},
            {-1, 0, 0},
            {-1, 0, -0},

            {0, -1, 0},
            {-0, -1, 0},
            {0, -1, 0},
            {-0, -1, 0},
            {0, -1, 0},
            {-0, -1, 0},

            {-0, 1, 0},
            {0, 1, 0},
            {-0, 1, 0},
            {0, 1, 0},
            {-0, 1, 0},
            {0, 1, 0}
        };
        constexpr glm::vec2 cubeUV[] = {
            // Front
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, -1.0f},
            {1.0f, -1.0f},
            {0.0f, -1.0f},
            {0.0f, 0.0f},

            // Right
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, -1.0f},
            {1.0f, -1.0f},
            {0.0f, -1.0f},
            {0.0f, 0.0f},

            // Back
            {1.0f, -1.0f},
            {0.0f, -1.0f},
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, -1.0f},

            // Left
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, -1.0f},
            {1.0f, -1.0f},
            {0.0f, -1.0f},
            {0.0f, 0.0f},

            // Bottom
            {0.0f, 0.0f},
            {-1.0f, 0.0f},
            {-1.0f, 1.0f},
            {-1.0f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 0.0f},

            // Top
            {0.0f, 0.0f},
            {-1.0f, 0.0f},
            {-1.0f, 1.0f},
            {-1.0f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 0.0f}
        };

        Mesh_F32 cubeMesh(3, 3 * sizeof(cubeVerticies) / sizeof(glm::vec3),
                          3 * sizeof(cubeNormals) / sizeof(glm::vec3),
                          2 * sizeof(cubeUV) / sizeof(glm::vec2), 0);

        std::memcpy(cubeMesh.GetVertexArray().data(), &cubeVerticies, sizeof(cubeVerticies));
        std::memcpy(cubeMesh.GetNormalsArray().data(), &cubeNormals, sizeof(cubeNormals));
        std::memcpy(cubeMesh.GetTexCoordsArray().data(), &cubeUV, sizeof(cubeUV));

        for (float &v: cubeMesh.GetVertexArray())
            v *= size;

        return cubeMesh;
    }

    //
    // //
    Mesh_F32 Geometry::CreateCircle(float radius, size_t segments) {
        //Two-dimensional vertices are used here
        size_t numberOfVerts = segments + 1;
        size_t numberOfIndices = segments * 3;
        Mesh_F32 circleMesh(2, numberOfVerts * 2, 0, 0,
                            numberOfIndices);


        std::vector<float> &vertices = circleMesh.GetVertexArray();
        std::vector<uint32_t> &indices = circleMesh.GetIndicesArray();

        vertices[0] = 0.0f;
        vertices[1] = 0.0f;

        for (size_t i = 1; i < segments + 1; i++) {
            float theta = 2.0f * std::numbers::pi_v<float> * float(i) / float(segments);

            vertices[i * 2 + 0] = radius * (float) cos(theta);
            vertices[i * 2 + 1] = radius * (float) sin(theta);
        }

        indices[0] = 0;
        indices[1] = 1;
        indices[2] = segments;

        for (size_t i = 1; i < numberOfIndices / 3; i++) {
            indices[i * 3 + 0] = 0;
            indices[i * 3 + 1] = i;
            indices[i * 3 + 2] = i + 1;
        }

        return circleMesh;
    }

    //
    // //
    Mesh_F32 Geometry::CreateRings(float innerRadius, float outerRadius, uint32_t segments) {
        uint32_t &faceCount = segments;
        size_t vertexCount = (size_t) faceCount * 4;
        size_t indexCount = (size_t) segments * 6;

        Mesh_F32 ringMesh(2, vertexCount, 0, 0, indexCount);

        std::vector<float> &Vertices = ringMesh.GetVertexArray();
        std::vector<uint32_t> &Indicies = ringMesh.GetIndicesArray();

        float theta;
        uint32_t i = 0;
        for (i = 0; i < faceCount; i++) {
            theta = 2.0f * Maths::PI * float(i) / float(segments);

            Vertices[(size_t) i * 4 + 0] = innerRadius * cos(theta);
            Vertices[(size_t) i * 4 + 1] = innerRadius * sin(theta);

            Vertices[(size_t) i * 4 + 2] = outerRadius * cos(theta);
            Vertices[(size_t) i * 4 + 3] = outerRadius * sin(theta);
        }

        for (i = 0; i < segments; i++) {
            if (i + 1 == segments) {
                Indicies[(size_t) i * 6 + 0] = i * 2 + 0;
                Indicies[(size_t) i * 6 + 1] = 1;
                Indicies[(size_t) i * 6 + 2] = i * 2 + 1;

                Indicies[(size_t) i * 6 + 3] = i * 2 + 0;
                Indicies[(size_t) i * 6 + 4] = 0;
                Indicies[(size_t) i * 6 + 5] = 1;
            } else {
                Indicies[(size_t) i * 6 + 0] = i * 2 + 0;
                Indicies[(size_t) i * 6 + 1] = i * 2 + 3;
                Indicies[(size_t) i * 6 + 2] = i * 2 + 1;

                Indicies[(size_t) i * 6 + 3] = i * 2 + 0;
                Indicies[(size_t) i * 6 + 4] = i * 2 + 2;
                Indicies[(size_t) i * 6 + 5] = i * 2 + 3;
            }
        }

        return ringMesh;
    }

    //
    // //
    Mesh_F32 Geometry::CreateHalfRings(float innerRadius, float outerRadius, uint32_t segments) {
        uint32_t faceCount = segments / 2 + 1;
        size_t vertexCount = (size_t) faceCount * 4;
        size_t indexCount = (size_t) (faceCount - 1) * 6;

        Mesh_F32 ringMesh(2, vertexCount, 0, 0, indexCount);

        std::vector<float> &Vertices = ringMesh.GetVertexArray();
        std::vector<uint32_t> &Indicies = ringMesh.GetIndicesArray();

        float theta;
        uint32_t i = 0;
        for (i = 0; i < faceCount; i++) {
            theta = 2.0f * Maths::PI * float(i) / float(segments);

            Vertices[(size_t) i * 4 + 0] = innerRadius * cos(theta);
            Vertices[(size_t) i * 4 + 1] = innerRadius * sin(theta);

            Vertices[(size_t) i * 4 + 2] = outerRadius * cos(theta);
            Vertices[(size_t) i * 4 + 3] = outerRadius * sin(theta);
        }

        for (i = 0; i < faceCount - 1; i++) {
            Indicies[(size_t) i * 6 + 0] = i * 2 + 0;
            Indicies[(size_t) i * 6 + 1] = i * 2 + 3;
            Indicies[(size_t) i * 6 + 2] = i * 2 + 1;

            Indicies[(size_t) i * 6 + 3] = i * 2 + 0;
            Indicies[(size_t) i * 6 + 4] = i * 2 + 2;
            Indicies[(size_t) i * 6 + 5] = i * 2 + 3;
        }

        return ringMesh;
    }

    //
    //
    Mesh_F32 Geometry::CreateTorus(float innerRadius, float outerRadius, float depth, uint32_t sides,
                                   uint32_t rings, uint32_t twistyness) {
        // Array for final verticies and normal vectors
        uint32_t elementCount = (sides + 1) * ((rings + 1) * (twistyness + 1));
        size_t vertexCount = ((size_t) elementCount * 3);
        size_t texCoordsCount = ((size_t) elementCount * 2);
        size_t indiciesCount = ((size_t) elementCount * 3) - 6;

        Mesh_F32 torusMesh(3,
                           vertexCount, vertexCount, texCoordsCount,
                           indiciesCount);

        std::vector<float> &Vertices = torusMesh.GetVertexArray();
        std::vector<float> &Normals = torusMesh.GetNormalsArray();
        std::vector<float> &TextureCoordinates = torusMesh.GetTexCoordsArray();
        std::vector<uint32_t> &Indicies = torusMesh.GetIndicesArray();

        // Incremental varibles
        uint32_t i = 0, j = 0;
        uint32_t a = 0;

        // Tesselation "twistyness". Varible used for correct quad tesselation
        short k = twistyness;
        // Angles for torus calculation
        float theta = 0, phi = 0;

        for (i = 0; i <= sides; i++) {
            for (j = 0; j <= rings; j++) {
                for (k = twistyness; k >= 0; k--) {
                    //	theta = (i + k) % rings+0.5f;
                    // phi = j % rings;
                    theta = (float) fmod(i + k, float(rings) + 0.5f);
                    phi = (float) fmod(j, rings);

                    Vertices[(size_t) a * 3 + 0] = (outerRadius + innerRadius * cos(theta * Maths::Tau / rings)) * cos(
                                                       phi * Maths::Tau / rings);
                    Vertices[(size_t) a * 3 + 1] = (outerRadius + innerRadius * cos(theta * Maths::Tau / rings)) * sin(
                                                       phi * Maths::Tau / rings);
                    Vertices[(size_t) a * 3 + 2] = depth * sin(theta * Maths::Tau / rings);

                    Normals[(size_t) a * 3 + 0] = cos(theta * Maths::Tau / rings) * cos(phi * Maths::Tau / rings);
                    Normals[(size_t) a * 3 + 1] = cos(theta * Maths::Tau / rings) * sin(phi * Maths::Tau / rings);
                    Normals[(size_t) a * 3 + 2] = sin(theta * Maths::Tau / rings);

                    //	TextureCoordinates.SetObjectValue(acos(currentVertex.y / innerRadius) / (2*phi), a * 2);
                    //	TextureCoordinates.SetObjectValue(acos(currentVertex.x/(innerRadius + outerRadius*cos(theta*TwoPI / rings)))*(2*phi), a * 2 + 1);
                    TextureCoordinates[(size_t) a * 2 + 0] = (i + k) / (float) sides;
                    TextureCoordinates[(size_t) a * 2 + 1] = phi / (float) rings;

                    a++;
                }
            }
        }

        j = (uint32_t) (indiciesCount / 6);
        for (i = 0; i < j; i++) {
            Indicies[(size_t) i * 6 + 0] = i * 2;
            Indicies[(size_t) i * 6 + 1] = i * 2 + 1;
            Indicies[(size_t) i * 6 + 2] = i * 2 + 2;

            Indicies[(size_t) i * 6 + 3] = i * 2 + 2;
            Indicies[(size_t) i * 6 + 4] = i * 2 + 1;
            Indicies[(size_t) i * 6 + 5] = i * 2 + 3;
        }

        return torusMesh;
    }

    //
    //
    std::vector<uint32_t> Geometry::DetectSphereWrappedUVCoords(const std::vector<uint32_t> &triangles,
                                                                const std::vector<float> &texCoords,
                                                                uint32_t hintCount) {
        std::vector<uint32_t> returnIndicies;
        returnIndicies.reserve(hintCount > 0 ? hintCount : 100);

        for (uint32_t i = 0; i < triangles.size() / 3; ++i) {
            const TriangleI currentTri = {triangles[i * 3 + 0], triangles[i * 3 + 1], triangles[i * 3 + 2]};
            glm::vec3 texA(texCoords[currentTri[0] * 2 + 0], texCoords[currentTri[0] * 2 + 1], 0.0f);
            glm::vec3 texB(texCoords[currentTri[1] * 2 + 0], texCoords[currentTri[1] * 2 + 1], 0.0f);
            glm::vec3 texC(texCoords[currentTri[2] * 2 + 0], texCoords[currentTri[2] * 2 + 1], 0.0f);

            glm::vec3 normal = glm::cross(texB - texA, texC - texA);
            if (normal.z > 0.0f) {
                returnIndicies.push_back(i);
            }
        }

        returnIndicies.shrink_to_fit();

        return returnIndicies;
    }

    //
    // //
    void Geometry::FixSphereWrappedUV(const std::vector<uint32_t> &wrapped, std::vector<float> &verticies,
                                      std::vector<uint32_t> &triangles, std::vector<float> &texCoords,
                                      uint32_t hintCount) {
        uint32_t vertexIndex = verticies.size() - 1;
        std::unordered_map<uint32_t, uint32_t> visited;

        if (hintCount > 0) {
            verticies.reserve(hintCount);
            texCoords.reserve(hintCount);
        }

        for (uint32_t i: wrapped) {
            uint32_t a = triangles[i * 3 + 0];
            uint32_t b = triangles[i * 3 + 1];
            uint32_t c = triangles[i * 3 + 2];

            if (texCoords[a * 2 + 0] < 0.25f) {
                uint32_t tempA = a;
                if (visited.find(a) == visited.end()) {
                    // glm::vec3 dVertex = verticies[a];
                    // verticies.push_back(dVertex);


                    verticies.push_back(verticies[a * 3 + 0]);
                    verticies.push_back(verticies[a * 3 + 1]);
                    verticies.push_back(verticies[a * 3 + 2]);

                    // glm::vec2 dTexCoord = texCoords[a];
                    // texCoords.emplace_back(dTexCoord.x + 1.0f, dTexCoord.y);

                    texCoords.push_back(texCoords[a * 2 + 0] + 1.0f);
                    texCoords.push_back(texCoords[a * 2 + 1]);

                    vertexIndex++;
                    visited.insert({a, vertexIndex});
                    tempA = vertexIndex;
                } else {
                    tempA = visited[a];
                }
                a = tempA;
            }

            if (texCoords[b * 2 + 0] < 0.25f) {
                uint32_t tempB = b;
                if (visited.find(b) == visited.end()) {
                    // glm::vec3 dVertex = verticies[a];
                    // verticies.push_back(dVertex);

                    verticies.push_back(verticies[a * 3 + 0]);
                    verticies.push_back(verticies[a * 3 + 1]);
                    verticies.push_back(verticies[a * 3 + 2]);

                    // glm::vec2 dTexCoord = texCoords[a];
                    // texCoords.emplace_back(dTexCoord.x + 1.0f, dTexCoord.y);

                    texCoords.push_back(texCoords[a * 2 + 0] + 1.0f);
                    texCoords.push_back(texCoords[a * 2 + 1]);

                    vertexIndex++;
                    visited.insert({b, vertexIndex});
                    tempB = vertexIndex;
                } else {
                    tempB = visited[b];
                }
                b = tempB;
            }

            if (texCoords[c * 2 + 0] < 0.25f) {
                uint32_t tempC = c;
                if (visited.find(b) == visited.end()) {
                    verticies.push_back(verticies[a * 3 + 0]);
                    verticies.push_back(verticies[a * 3 + 1]);
                    verticies.push_back(verticies[a * 3 + 2]);

                    // glm::vec2 dTexCoord = texCoords[a];
                    // texCoords.emplace_back(dTexCoord.x + 1.0f, dTexCoord.y);

                    texCoords.push_back(texCoords[a * 2 + 0] + 1.0f);
                    texCoords.push_back(texCoords[a * 2 + 1]);

                    vertexIndex++;
                    visited.insert({c, vertexIndex});
                    tempC = vertexIndex;
                } else {
                    tempC = visited[c];
                }
                c = tempC;
            }

            triangles[i * 3 + 0] = a;
            triangles[i * 3 + 1] = b;
            triangles[i * 3 + 2] = c;
        }
    }

    //
    // //
    // void Geometry::FixPoleVerticies(std::vector<glm::vec3> &verticies, std::vector<glm::vec2> &texCoords,
    //                                 std::vector<TriangleI> &triangles) {
    //     uint32_t northPole = 0, southPole = 0;
    //     uint32_t i = 0;
    //
    //     for (; i < verticies.size(); i++) {
    //         if (verticies[i].y >= 0.9995f)
    //             northPole = i;
    //         else if (verticies[i].y <= -0.9995f)
    //             southPole = i;
    //     }
    //
    //     uint32_t verticeIndex = (uint32_t) verticies.size() - 1;
    //
    //     auto FixVertex = [&triangles, &texCoords, &verticies, &verticeIndex](
    //         uint32_t i, uint32_t poleIndex, uint32_t b, uint32_t c) -> void {
    //         const TriangleI &currentTri = triangles[i];
    //         const glm::vec2 &B = texCoords[currentTri[b]];
    //         const glm::vec2 &C = texCoords[currentTri[c]];
    //         verticies.push_back(verticies[currentTri[poleIndex]]);
    //         // texCoords.emplace_back((B.x + C.x) * 0.5f, texCoords[currentTri[poleIndex]].y);
    //         texCoords.emplace_back(1, 1);
    //         verticeIndex++;
    //         triangles[i][poleIndex] = verticeIndex;
    //     };
    //
    //     for (i = 0; i < triangles.size(); i++) {
    //         if (triangles[i].x == northPole) {
    //             FixVertex(i, 0, 1, 2);
    //         } else if (triangles[i].y == northPole) {
    //             FixVertex(i, 1, 0, 2);
    //         } else if (triangles[i].z == northPole) {
    //             FixVertex(i, 2, 0, 1);
    //         }
    //
    //         if (triangles[i].x == southPole) {
    //             FixVertex(i, 0, 1, 2);
    //         } else if (triangles[i].y == southPole) {
    //             FixVertex(i, 1, 0, 2);
    //         } else if (triangles[i].z == southPole) {
    //             FixVertex(i, 2, 0, 1);
    //         }
    //     }
    // }

    /*
    private void FixSharedPoleVertices(Mesh sphereMesh)
    {
    Vertex north = sphereMesh.Vertices.First((v) => v.Position.Y == 1);
    Vertex south = sphereMesh.Vertices.First((v) => v.Position.Y == -1);
    uint northIndex = (uint)Array.IndexOf(sphereMesh.Vertices, north);
    uint southIndex = (uint)Array.IndexOf(sphereMesh.Vertices, south);
    List<Vertex> vertices = new List<Vertex>(sphereMesh.Vertices);
    uint verticeIndex = (uint)sphereMesh.Vertices.Length - 1;
    for (int i = 0; i < sphereMesh.Triangles.Length; ++i)
    {
        if (sphereMesh.Triangles[i].A == northIndex)
        {
            Vertex A = sphereMesh.Vertices[sphereMesh.Triangles[i].A];
            Vertex B = sphereMesh.Vertices[sphereMesh.Triangles[i].B];
            Vertex C = sphereMesh.Vertices[sphereMesh.Triangles[i].C];
            Vertex newNorth = north;
            newNorth.TexCoord.X = (B.TexCoord.X + C.TexCoord.X) / 2;
            verticeIndex++;
            vertices.Add(newNorth);
            sphereMesh.Triangles[i].A = verticeIndex;
        }
        else if (sphereMesh.Triangles[i].A == southIndex)
        {
            Vertex A = sphereMesh.Vertices[sphereMesh.Triangles[i].A];
            Vertex B = sphereMesh.Vertices[sphereMesh.Triangles[i].B];
            Vertex C = sphereMesh.Vertices[sphereMesh.Triangles[i].C];
            Vertex newSouth = south;
            newSouth.TexCoord.X = (B.TexCoord.X + C.TexCoord.X) / 2;
            verticeIndex++;
            vertices.Add(newSouth);
            sphereMesh.Triangles[i].A = verticeIndex;
        }
    }
    sphereMesh.Vertices = vertices.ToArray();
    }
    */
    //
#pragma endregion
    //
}
