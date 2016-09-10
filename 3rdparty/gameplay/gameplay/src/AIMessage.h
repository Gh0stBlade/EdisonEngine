#pragma once

namespace gameplay
{
    /**
     * Defines a simple message structure used for passing messages through
     * the AI system.
     *
     * Messages can store an arbitrary number of parameters. For the sake of simplicity,
     * each parameter is stored as type double, which is flexible enough to store most
     * data that needs to be passed.
     */
    class AIMessage
    {
        friend class AIAgent;
        friend class AIController;
        friend class AIStateMachine;

    public:
        explicit AIMessage();
        ~AIMessage();

        /**
         * Enumeration of supported AIMessage parameter types.
         */
        enum ParameterType
        {
            UNDEFINED,
            INTEGER,
            LONG,
            FLOAT,
            DOUBLE,
            BOOLEAN,
            STRING
        };


        /**
         * Creates a new message.
         *
         * Once a message is constructed and populated with data, it can be routed to its
         * intended recipient(s) by calling AIController::sendMessage(AIMessage*). The
         * AIController will then handle scheduling and delivery of the message and it will
         * also destroy the message after it has been successfully delivered. For this reason,
         * once a message has been sent through AIController, it is unsafe to use or destroy
         * the message pointer.
         *
         * @param id The message ID.
         * @param sender AIAgent sender ID (can be empty or null for an anonymous message).
         * @param receiver AIAgent receiver ID (can be empty or null for a broadcast message).
         * @param parameterCount Number of parameters for this message.
         *
         * @return A new AIMessage.
         */
        static std::shared_ptr<AIMessage> create(unsigned int id, const std::string& sender, const std::string& receiver, unsigned int parameterCount);

        /**
         * Returns the message ID.
         *
         * @return The message ID.
         */
        unsigned int getId() const;

        /**
         * Returns the sender for the message.
         *
         * @return The message sender ID.
         */
        const char* getSender() const;

        /**
         * Returns the receiver for the message.
         *
         * @return The message receiver.
         */
        const char* getReceiver() const;

        /**
         * Returns the value of the specified parameter as an integer.
         *
         * @param index Index of the parameter to get.
         *
         * @return The parameter value.
         */
        int getInt(unsigned int index) const;

        /**
         * Sets an integer parameter.
         *
         * @param index Index of the parameter to set.
         * @param value The parameter value.
         */
        void setInt(unsigned int index, int value);

        /**
         * Returns the value of the specified parameter as a long integer.
         *
         * @param index Index of the parameter to get.
         *
         * @return The parameter value.
         */
        long getLong(unsigned int index) const;

        /**
         * Sets a long integer parameter.
         *
         * @param index Index of the parameter to set.
         * @param value The parameter value.
         */
        void setLong(unsigned int index, long value);

        /**
         * Returns the value of the specified parameter as a float.
         *
         * @param index Index of the parameter to get.
         *
         * @return The parameter value.
         */
        float getFloat(unsigned int index) const;

        /**
         * Sets a float parameter.
         *
         * @param index Index of the parameter to set.
         * @param value The parameter value.
         */
        void setFloat(unsigned int index, float value);

        /**
         * Returns the value of the specified parameter as a double.
         *
         * @param index Index of the parameter to get.
         *
         * @return The parameter value.
         */
        double getDouble(unsigned int index) const;

        /**
         * Sets a double parameter.
         *
         * @param index Index of the parameter to set.
         * @param value The parameter value.
         */
        void setDouble(unsigned int index, double value);

        /**
         * Returns the value of the specified parameter as a boolean.
         *
         * @param index Index of the parameter to get.
         *
         * @return The parameter value.
         */
        bool getBoolean(unsigned int index) const;

        /**
         * Sets a long parameter.
         *
         * @param index Index of the parameter to set.
         * @param value The parameter value.
         */
        void setBoolean(unsigned int index, bool value);

        /**
         * Returns the value of the specified parameter as a string.
         *
         * @param index Index of the parameter to get.
         *
         * @return The parameter value.
         */
        const char* getString(unsigned int index) const;

        /**
         * Sets a string parameter.
         *
         * @param index Index of the parameter to set.
         * @param value The parameter value.
         */
        void setString(unsigned int index, const char* value);

        /**
         * Returns the number of parameters for this message.
         *
         * @return The number of message parameters.
         */
        unsigned int getParameterCount() const;

        /**
         * Returns the type of the specified parameter.
         *
         * @param index Index of the parameter to query.
         *
         * @return The parameter type.
         */
        ParameterType getParameterType(unsigned int index) const;

    private:

        /**
         * Internal message type enumeration.
         */
        enum MessageType
        {
            MESSAGE_TYPE_STATE_CHANGE,
            MESSAGE_TYPE_CUSTOM
        };


        /**
         * Defines a flexible message parameter.
         */
        struct Parameter
        {
            Parameter();

            ~Parameter();

            void clear();


            union
            {
                int intValue;
                long longValue;
                float floatValue;
                double doubleValue;
                bool boolValue;
                char* stringValue;
            };


            AIMessage::ParameterType type;
        };


        AIMessage(const AIMessage&) = delete;

        AIMessage& operator=(const AIMessage&) = delete;

        /**
         * Returns the delivery time for the message.
         *
         * @return The delivery time for the message, or zero if the message is not currently scheduled to be delivered.
         */
        std::chrono::microseconds getDeliveryTime() const;

        void clearParameter(unsigned int index);

        unsigned int _id;
        std::string _sender;
        std::string _receiver;
        std::chrono::microseconds _deliveryTime;
        Parameter* _parameters;
        unsigned int _parameterCount;
        MessageType _messageType;
    };
}
