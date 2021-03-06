#pragma once

#include "gl/program.h"

#include <map>
#include <memory>
#include <vector>


namespace gameplay
{
    class ShaderProgram
    {
    public:
        explicit ShaderProgram();

        ~ShaderProgram();

        static std::shared_ptr<ShaderProgram> createFromFile(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines = {});

        const std::string& getId() const;

        const gl::Program::ActiveAttribute* getVertexAttribute(const std::string& name) const;

        gl::Program::ActiveUniform* getUniform(const std::string& name) const;

        gl::Program::ActiveUniform* getUniform(size_t index) const;

        size_t getUniformCount() const;

        void bind();


        const gl::Program& getHandle() const
        {
            return m_handle;
        }


    private:

        ShaderProgram(const ShaderProgram&) = delete;

        ShaderProgram& operator=(const ShaderProgram&) = delete;

        static std::shared_ptr<ShaderProgram> createFromSource(const std::string& vshPath, const std::string& vshSource, const std::string& fshPath, const std::string& fshSource, const std::vector<std::string>& defines = {});

        std::string _id;

        gl::Program m_handle;

        std::map<std::string, gl::Program::ActiveAttribute> _vertexAttributes;

        mutable std::map<std::string, gl::Program::ActiveUniform> _uniforms;
    };
}
