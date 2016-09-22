#pragma once

#include "core/coordinates.h"
#include "primitives.h"
#include "texture.h"
#include "util.h"

#include <vector>


namespace render
{
    class TextureAnimator;
}


namespace loader
{
    struct Mesh
    {
        core::TRCoordinates center; // This is usually close to the mesh's centroid, and appears to be the center of a sphere used for collision testing.
        int32_t collision_size; // This appears to be the radius of that aforementioned collisional sphere.
        std::vector<core::TRCoordinates> vertices; //[NumVertices]; // list of vertices (relative coordinates)
        std::vector<core::TRCoordinates> normals; //[NumNormals]; // list of normals (if NumNormals is positive)
        std::vector<int16_t> lights; //[-NumNormals]; // list of light values (if NumNormals is negative)
        std::vector<QuadFace> textured_rectangles; //[NumTexturedRectangles]; // list of textured rectangles
        std::vector<Triangle> textured_triangles; //[NumTexturedTriangles]; // list of textured triangles
        // the rest is not present in TR4
        std::vector<QuadFace> colored_rectangles; //[NumColouredRectangles]; // list of coloured rectangles
        std::vector<Triangle> colored_triangles; //[NumColouredTriangles]; // list of coloured triangles

        /** \brief reads mesh definition.
        *
        * The read num_normals value is positive when normals are available and negative when light
        * values are available. The values get set appropiatly.
        */
        static std::unique_ptr<Mesh> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<Mesh> mesh{new Mesh()};
            mesh->center = readCoordinates16(reader);
            mesh->collision_size = reader.readI32();

            reader.readVector(mesh->vertices, reader.readI16(), &io::readCoordinates16);

            auto num_normals = reader.readI16();
            if( num_normals >= 0 )
            {
                reader.readVector(mesh->normals, num_normals, &io::readCoordinates16);
            }
            else
            {
                reader.readVector(mesh->lights, -num_normals);
            }

            reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr1);
            reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr1);
            reader.readVector(mesh->colored_rectangles, reader.readU16(), &QuadFace::readTr1);
            reader.readVector(mesh->colored_triangles, reader.readU16(), &Triangle::readTr1);

            return mesh;
        }


        static std::unique_ptr<Mesh> readTr4(io::SDLReader& reader)
        {
            std::unique_ptr<Mesh> mesh{new Mesh()};
            mesh->center = readCoordinates16(reader);
            mesh->collision_size = reader.readI32();

            reader.readVector(mesh->vertices, reader.readU16(), &io::readCoordinates16);

            auto num_normals = reader.readI16();
            if( num_normals >= 0 )
            {
                reader.readVector(mesh->normals, num_normals, &io::readCoordinates16);
            }
            else
            {
                reader.readVector(mesh->lights, -num_normals);
            }

            reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr4);
            reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr4);

            return mesh;
        }


        class ModelBuilder
        {
            struct RenderVertex;
            struct RenderVertexWithNormal;

            const bool m_hasNormals;
            const bool m_withWeights;
            std::vector<float> m_vbuf;
            const std::vector<TextureLayoutProxy>& m_textureProxies;
            const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& m_materials;
            const std::vector<std::shared_ptr<gameplay::Material>>& m_colorMaterials;
            render::TextureAnimator& m_animator;
            std::map<TextureLayoutProxy::TextureKey, size_t> m_texBuffers;
            size_t m_vertexCount = 0;
            std::shared_ptr<gameplay::Mesh> m_mesh;

            static gameplay::VertexFormat getFormat(bool withNormals, bool withWeights);

            struct MeshPart
            {
                using IndexBuffer = std::vector<uint16_t>;

                IndexBuffer indices;
                std::shared_ptr<gameplay::Material> material;
            };


            std::vector<MeshPart> m_parts;

            void append(const RenderVertex& v);
            void append(const RenderVertexWithNormal& v);


            size_t getPartForColor(uint16_t proxyId)
            {
                const TextureLayoutProxy& proxy = m_textureProxies.at(proxyId);

                TextureLayoutProxy::TextureKey tk;
                tk.blendingMode = BlendingMode::Solid;
                tk.flags = 0;
                tk.tileAndFlag = 0;
                tk.colorId = proxyId & 0xff;

                if( m_texBuffers.find(tk) == m_texBuffers.end() )
                {
                    m_texBuffers[tk] = m_parts.size();
                    m_parts.emplace_back();
                    m_parts.back().material = m_colorMaterials[tk.colorId];
                }

                return m_texBuffers[tk];
            }


            size_t getPartForTexture(const TextureLayoutProxy& proxy)
            {
                if( m_texBuffers.find(proxy.textureKey) == m_texBuffers.end() )
                {
                    m_texBuffers[proxy.textureKey] = m_parts.size();
                    m_parts.emplace_back();
                    auto it = m_materials.find(proxy.textureKey);
                    Expects(it != m_materials.end());
                    m_parts.back().material = it->second;
                }
                return m_texBuffers[proxy.textureKey];
            }


        public:
            explicit ModelBuilder(bool withNormals,
                                  bool dynamic,
                                  bool withWeights,
                                  const std::vector<TextureLayoutProxy>& textureProxies,
                                  const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& materials,
                                  const std::vector<std::shared_ptr<gameplay::Material>>& colorMaterials,
                                  render::TextureAnimator& animator);
            ~ModelBuilder();


            void append(const Mesh& mesh, float blendWeight = 0, float blendIndex = 0);

            std::shared_ptr<gameplay::Model> finalize();
        };


        std::shared_ptr<gameplay::Model> createModel(const std::vector<TextureLayoutProxy>& textureProxies,
                                                     const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& materials,
                                                     const std::vector<std::shared_ptr<gameplay::Material>>& colorMaterials,
                                                     render::TextureAnimator& animator) const;
    };
}
