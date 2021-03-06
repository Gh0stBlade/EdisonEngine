#pragma once

#include "ShaderProgram.h"

#include <glm/glm.hpp>

#include <boost/optional.hpp>


namespace gameplay
{
    class Node;


    /**
     * Defines a material parameter.
     *
     * This class represents a parameter that can be set for a material.
     * The methods in this class provide a mechanism to set parameters
     * of all supported types. Some types support setting by value,
     * while others only support setting by reference/pointer.
     *
     * Setting a parameter by reference/pointer provides the ability to
     * pass an array of values as well as a convenient way to support
     * auto-binding of values to a material parameter. For example, by
     * setting the parameter value to a pointer to a glm::mat4, any changes
     * to the glm::mat4 will automatically be reflected in the technique the
     * next time the parameter is applied to the render state.
     *
     * Note that for parameter values to arrays or pointers, the
     * MaterialParameter will keep a long-lived reference to the passed
     * in array/pointer. Therefore, you must ensure that the pointers
     * you pass in are valid for the lifetime of the MaterialParameter
     * object.
     */
    class MaterialParameter
    {
        friend class RenderState;

    public:
        explicit MaterialParameter(const std::string& name);
        ~MaterialParameter();

        /**
         * Returns the name of this material parameter.
         */
        const std::string& getName() const;

        /**
         * Sets the value of this parameter to a float value.
         */
        void set(float value);

        /**
         * Sets the value of this parameter to an integer value.
         */
        void set(int value);

        /**
         * Stores a pointer/array of float values in this parameter.
         */
        void set(const float* values, size_t count = 1);

        /**
         * Stores a pointer/array of integer values in this parameter.
         */
        void set(const int* values, size_t count = 1);

        /**
         * Stores a copy of the specified glm::vec2 value in this parameter.
         */
        void set(const glm::vec2& value);

        /**
         * Stores a pointer/array of glm::vec2 values in this parameter.
         */
        void set(const glm::vec2* values, size_t count = 1);

        /**
         * Stores a copy of the specified glm::vec3 value in this parameter.
         */
        void set(const glm::vec3& value);

        /**
         * Stores a pointer/array of glm::vec3 values in this parameter.
         */
        void set(const glm::vec3* values, size_t count = 1);

        /**
         * Stores a copy of the specified glm::vec4 value in this parameter.
         */
        void set(const glm::vec4& value);

        /**
         * Stores a pointer/array of glm::vec4 values in this parameter.
         */
        void set(const glm::vec4* values, size_t count = 1);

        /**
         * Stores a copy of the specified glm::mat4 value in this parameter.
         */
        void set(const glm::mat4& value);

        /**
         * Stores a pointer/array of glm::mat4 values in this parameter.
         */
        void set(const glm::mat4* values, size_t count = 1);

        /**
         * Sets the value of this parameter to the specified texture sampler.
         */
        void set(const std::shared_ptr<gl::Texture>& texture);

        /**
         * Sets the value of this parameter to the specified texture sampler array.
         *
         * @script{ignore}
         */
        void set(const std::vector<std::shared_ptr<gl::Texture>>& textures);


        /**
         * Binds the return value of a class method to this material parameter.
         *
         * This method enables binding of arbitrary class methods to a material
         * parameter. This is useful when you want to set a material parameter
         * to a variable that is frequently changing (such as a world matrix).
         *
         * By binding a method pointer, the method will be called automatically
         * to retrieve the updated parameter value each time the material is bound
         * for rendering.
         *
         * @param classInstance The instance of the class containing the member method to bind.
         * @param valueMethod A pointer to the class method to bind (in the format '&class::method').
         */
        template<class ClassType, class ValueType>
        void bind(ClassType* classInstance, ValueType (ClassType::*valueMethod)() const)
        {
            m_valueSetter = [classInstance, valueMethod](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
                {
                    uniform.set((classInstance ->* valueMethod)());
                };
        }

        using UniformValueSetter = void(const Node& node, gl::Program::ActiveUniform& uniform);

        void bind(std::function<UniformValueSetter>&& setter)
        {
            m_valueSetter = std::move(setter);
        }

        /**
         * Binds the return value of a class method to this material parameter.
         *
         * This overloads the setBinding method to provide support for array parameters.
         * The valueMethod parameter should return an array (pointer) of a supported
         * material parameter type, such as glm::mat4* for an array of matrices. The
         * countMethod should point to a method that returns the number of entries in
         * the value returned from valueMethod.
         *
         * @param classInstance The instance of the class containing the member method to bind.
         * @param valueMethod A pointer to the class method to bind (in the format '&class::method').
         * @param countMethod A pointer to a method that returns the number of entries in the array returned by valueMethod.
         */
        template<class ClassType, class ValueType>
        void bind(ClassType* classInstance, ValueType (ClassType::*valueMethod)() const, size_t (ClassType::*countMethod)() const)
        {
            m_valueSetter = [classInstance, valueMethod, countMethod](const Node& /*node*/, const gl::Program::ActiveUniform& uniform)
                {
                    uniform.set((classInstance ->* valueMethod)(), (classInstance ->* countMethod)());
                };
        }

        void bindWorldViewProjectionMatrix();
        void bindModelMatrix();
        void bindViewMatrix();

    private:

        MaterialParameter& operator=(const MaterialParameter&) = delete;

        void bind(const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram);

        gl::Program::ActiveUniform* getUniform(const std::shared_ptr<ShaderProgram>& shaderProgram);


        enum LOGGER_DIRTYBITS
        {
            UNIFORM_NOT_FOUND = 0x01,
            PARAMETER_VALUE_NOT_SET = 0x02
        };

        const std::string m_name;
        boost::optional<std::function<UniformValueSetter>> m_valueSetter;
        uint8_t m_loggerDirtyBits = 0;
    };
}
