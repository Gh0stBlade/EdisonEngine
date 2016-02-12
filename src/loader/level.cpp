/*
 * Copyright 2002 - Florian Schulze <crow@icculus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This file is part of vt.
 *
 */

#include "level.h"
#include "tr1level.h"
#include "tr2level.h"
#include "tr3level.h"
#include "tr4level.h"
#include "tr5level.h"

#include <algorithm>
#include <boost/lexical_cast.hpp>

using namespace loader;

/// \brief reads the mesh data.
void Level::readMeshData(io::SDLReader& reader)
{
    uint32_t meshDataWords = reader.readU32();
    const auto basePos = reader.tell();

    const auto meshDataSize = meshDataWords * 2;
    reader.skip(meshDataSize);

    reader.readVector(m_meshIndices, reader.readU32());
    const auto endPos = reader.tell();

    m_meshes.clear();

    size_t meshDataPos = 0;
    for(size_t i = 0; i < m_meshIndices.size(); i++)
    {
        std::replace(m_meshIndices.begin(), m_meshIndices.end(), meshDataPos, i);

        reader.seek(basePos + meshDataPos);

        if(gameToEngine(m_gameVersion) >= Engine::TR4)
            m_meshes.emplace_back(*Mesh::readTr4(reader));
        else
            m_meshes.emplace_back(*Mesh::readTr1(reader));

        for(size_t j = 0; j < m_meshIndices.size(); j++)
        {
            if(m_meshIndices[j] > meshDataPos)
            {
                meshDataPos = m_meshIndices[j];
                break;
            }
        }
    }

    reader.seek(endPos);
}

/// \brief reads frame and moveable data.
void Level::readPoseDataAndModels(io::SDLReader& reader)
{
    m_poseData.resize(reader.readU32());
    reader.readVector(m_poseData, m_poseData.size());

    m_animatedModels.resize(reader.readU32());
    for(std::unique_ptr<AnimatedModel>& model : m_animatedModels)
    {
        if(gameToEngine(m_gameVersion) < Engine::TR5)
        {
            model = AnimatedModel::readTr1(reader);
            // Disable unused skybox polygons.
            if(gameToEngine(m_gameVersion) == Engine::TR3 && model->object_id == 355)
            {
                m_meshes[m_meshIndices[model->firstMesh]].colored_triangles.resize(16);
            }
        }
        else
        {
            model = AnimatedModel::readTr5(reader);
        }
    }
}

std::unique_ptr<Level> Level::createLoader(const std::string& filename, Game game_version)
{
    size_t len2 = 0;

    for(size_t i = 0; i < filename.length(); i++)
    {
        if(filename[i] == '/' || filename[i] == '\\')
        {
            len2 = i;
        }
    }

    std::string sfxPath;

    if(len2 > 0)
    {
        sfxPath = filename.substr(0, len2 + 1) + "MAIN.SFX";
    }
    else
    {
        sfxPath = "MAIN.SFX";
    }

    io::SDLReader reader(filename);
    if(!reader.isOpen())
        return nullptr;

    if(game_version == Game::Unknown)
        game_version = probeVersion(reader, filename);
    if(game_version == Game::Unknown)
        return nullptr;

    reader.seek(0);
    return createLoader(std::move(reader), game_version, sfxPath);
}

/** \brief reads the level.
  *
  * Takes a SDL_RWop and the game_version of the file and reads the structures into the members of TR_Level.
  */
std::unique_ptr<Level> Level::createLoader(io::SDLReader&& reader, Game game_version, const std::string& sfxPath)
{
    if(!reader.isOpen())
        return nullptr;

    std::unique_ptr<Level> result;

    switch(game_version)
    {
        case Game::TR1:
            result.reset(new TR1Level(game_version, std::move(reader)));
            break;
        case Game::TR1Demo:
        case Game::TR1UnfinishedBusiness:
            result.reset(new TR1Level(game_version, std::move(reader)));
            result->m_demoOrUb = true;
            break;
        case Game::TR2:
            result.reset(new TR2Level(game_version, std::move(reader)));
            break;
        case Game::TR2Demo:
            result.reset(new TR2Level(game_version, std::move(reader)));
            result->m_demoOrUb = true;
            break;
        case Game::TR3:
            result.reset(new TR3Level(game_version, std::move(reader)));
            break;
        case Game::TR4:
        case Game::TR4Demo:
            result.reset(new TR4Level(game_version, std::move(reader)));
            break;
        case Game::TR5:
            result.reset(new TR5Level(game_version, std::move(reader)));
            break;
        default:
            BOOST_THROW_EXCEPTION(std::runtime_error("Invalid game version"));
    }

    result->m_sfxPath = sfxPath;
    return result;
}

Game Level::probeVersion(io::SDLReader& reader, const std::string& filename)
{
    if(!reader.isOpen() || filename.length() < 5)
        return Game::Unknown;

    std::string ext;
    ext += filename[filename.length() - 4];
    ext += toupper(filename[filename.length() - 3]);
    ext += toupper(filename[filename.length() - 2]);
    ext += toupper(filename[filename.length() - 1]);

    reader.seek(0);
    uint8_t check[4];
    reader.readBytes(check, 4);

    Game ret = Game::Unknown;
    if(ext == ".PHD")
    {
        if(check[0] == 0x20 &&
           check[1] == 0x00 &&
           check[2] == 0x00 &&
           check[3] == 0x00)
        {
            ret = Game::TR1;
        }
    }
    else if(ext == ".TUB")
    {
        if(check[0] == 0x20 &&
           check[1] == 0x00 &&
           check[2] == 0x00 &&
           check[3] == 0x00)
        {
            ret = loader::Game::TR1UnfinishedBusiness;
        }
    }
    else if(ext == ".TR2")
    {
        if(check[0] == 0x2D &&
           check[1] == 0x00 &&
           check[2] == 0x00 &&
           check[3] == 0x00)
        {
            ret = loader::Game::TR2;
        }
        else if((check[0] == 0x38 || check[0] == 0x34) &&
                check[1] == 0x00 &&
                (check[2] == 0x18 || check[2] == 0x08) &&
                check[3] == 0xFF)
        {
            ret = loader::Game::TR3;
        }
    }
    else if(ext == ".TR4")
    {
        if(check[0] == 0x54 &&                                         // T
           check[1] == 0x52 &&                                         // R
           check[2] == 0x34 &&                                         // 4
           check[3] == 0x00)
        {
            ret = loader::Game::TR4;
        }
        else if(check[0] == 0x54 &&                                         // T
                check[1] == 0x52 &&                                         // R
                check[2] == 0x34 &&                                         // 4
                check[3] == 0x63)                                           //
        {
            ret = loader::Game::TR4;
        }
        else if(check[0] == 0xF0 &&                                         // T
                check[1] == 0xFF &&                                         // R
                check[2] == 0xFF &&                                         // 4
                check[3] == 0xFF)
        {
            ret = loader::Game::TR4;
        }
    }
    else if(ext == ".TRC")
    {
        if(check[0] == 0x54 &&                                              // T
           check[1] == 0x52 &&                                              // R
           check[2] == 0x34 &&                                              // C
           check[3] == 0x00)
        {
            ret = loader::Game::TR5;
        }
    }

    return ret;
}

StaticMesh *Level::findStaticMeshById(uint32_t object_id)
{
    for(size_t i = 0; i < m_staticMeshes.size(); i++)
        if(m_staticMeshes[i].object_id == object_id && m_meshIndices[m_staticMeshes[i].mesh])
            return &m_staticMeshes[i];

    return nullptr;
}

int Level::findMeshIndexByObjectId(uint32_t object_id) const
{
    for(size_t i = 0; i < m_staticMeshes.size(); i++)
        if(m_staticMeshes[i].object_id == object_id)
            return m_meshIndices[m_staticMeshes[i].mesh];
    
    return -1;
}

Item *Level::findItemById(int32_t object_id)
{
    for(size_t i = 0; i < m_items.size(); i++)
        if(m_items[i].object_id == object_id)
            return &m_items[i];

    return nullptr;
}

AnimatedModel* Level::findModelById(uint32_t object_id)
{
    for(size_t i = 0; i < m_animatedModels.size(); i++)
        if(m_animatedModels[i]->object_id == object_id)
            return m_animatedModels[i].get();

    return nullptr;
}

std::vector<irr::video::ITexture*> Level::createTextures(irr::video::IVideoDriver* drv)
{
    BOOST_ASSERT(!m_textures.empty());
    std::vector<irr::video::ITexture*> textures;
    for(int i=0; i<m_textures.size(); ++i)
    {
        DWordTexture& texture = m_textures[i];
        textures.emplace_back(texture.toTexture(drv, i));
    }
    return textures;
}

void Level::toIrrlicht(irr::scene::ISceneManager* mgr)
{
    std::vector<irr::video::ITexture*> textures = createTextures(mgr->getVideoDriver());
    std::map<UVTexture::TextureKey, irr::video::SMaterial> materials;
    const auto texMask = gameToEngine(m_gameVersion)==Engine::TR4 ? TextureIndexMaskTr4 : TextureIndexMask;
    for(UVTexture& uvTexture : m_uvTextures)
    {
        const auto& key = uvTexture.textureKey;
        if(materials.find(key) != materials.end())
            continue;
        
        materials[key] = UVTexture::createMaterial(textures[key.tileAndFlag & texMask], key.blendingMode);
    }
    std::vector<irr::video::SMaterial> coloredMaterials;
    std::vector<irr::scene::SMesh*> staticMeshes;
    for(size_t i=0; i<m_meshes.size(); ++i)
    {
        staticMeshes.emplace_back(m_meshes[i].createMesh(mgr, i, m_uvTextures, materials, coloredMaterials));
    }

    irr::core::vector3df cpos;
    for(size_t i=0; i<m_rooms.size(); ++i)
    {
        irr::scene::IMeshSceneNode* n = m_rooms[i].createSceneNode(mgr, i, *this, materials, staticMeshes);
        if(i==0)
            cpos = n->getAbsolutePosition();
    }
    
    irr::SKeyMap keyMap[7];
    keyMap[0].Action = irr::EKA_MOVE_FORWARD;
    keyMap[0].KeyCode = irr::KEY_KEY_W;

    keyMap[1].Action = irr::EKA_MOVE_BACKWARD;
    keyMap[1].KeyCode = irr::KEY_KEY_S;

    keyMap[2].Action = irr::EKA_STRAFE_LEFT;
    keyMap[2].KeyCode = irr::KEY_KEY_A;

    keyMap[3].Action = irr::EKA_STRAFE_RIGHT;
    keyMap[3].KeyCode = irr::KEY_KEY_D;

    keyMap[4].Action = irr::EKA_JUMP_UP;
    keyMap[4].KeyCode = irr::KEY_SPACE;

    keyMap[5].Action = irr::EKA_CROUCH;
    keyMap[5].KeyCode = irr::KEY_SHIFT;
    keyMap[6].Action = irr::EKA_CROUCH;
    keyMap[6].KeyCode = irr::KEY_CONTROL;

    irr::scene::ICameraSceneNode* camera = mgr->addCameraSceneNodeFPS(nullptr, 50.f, 10.0f, -1, keyMap, 7, false, 10.0f);
    camera->setNearValue(1);
    camera->setFarValue(2e5);
    
    camera->setPosition(cpos);
}

void Level::convertTexture(ByteTexture & tex, Palette & pal, DWordTexture & dst)
{
    for(int y = 0; y < 256; y++)
    {
        for(int x = 0; x < 256; x++)
        {
            int col = tex.pixels[y][x];
            
            if(col > 0)
                dst.pixels[y][x].set(0xff, pal.color[col].r, pal.color[col].g, pal.color[col].b);
            else
                dst.pixels[y][x].set(0);
        }
    }
}

void Level::convertTexture(WordTexture & tex, DWordTexture & dst)
{
    for(int y = 0; y < 256; y++)
    {
        for(int x = 0; x < 256; x++)
        {
            int col = tex.pixels[y][x];
            
            if(col & 0x8000)
            {
                const uint32_t a = 0xff;
                const uint32_t r = ((col & 0x00007c00) >> 7);
                const uint32_t g = ((col & 0x000003e0) >> 2);
                const uint32_t b = ((col & 0x0000001f) << 3);
                dst.pixels[y][x].set(a,r,g,b);
            }
            else
            {
                dst.pixels[y][x].set(0);
            }
        }
    }
}
