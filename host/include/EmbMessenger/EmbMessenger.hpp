#ifndef EMBMESSENGER_EMBMESSENGER_HPP
#define EMBMESSENGER_EMBMESSENGER_HPP

#include "EmbMessenger/Command.hpp"
#include "EmbMessenger/IBuffer.hpp"
#include "EmbMessenger/Reader.hpp"
#include "EmbMessenger/Writer.hpp"

#include <map>
#include <memory>
#include <typeindex>

namespace emb
{
    class EmbMessenger
    {
    protected:
        IBuffer* m_buffer;
        Writer m_writer;
        Reader m_reader;

        uint16_t m_message_id;
        std::map<std::type_index, uint8_t> m_command_ids;
        std::map<uint16_t, std::shared_ptr<Command>> m_commands;

        void write();
        void read();

    public:
        EmbMessenger(IBuffer* buffer);

        template <typename T>
        void registerCommand(const uint8_t id)
        {
            m_command_ids.emplace(typeid(T), id);
        }

        template <typename CommandType>
        std::shared_ptr<CommandType> send(std::shared_ptr<CommandType> command)
        {
            command->m_message_id = m_message_id;
            m_commands.emplace(m_message_id, command);

            write(m_message_id++, m_command_ids.at(typeid(CommandType)));
            command->send(this);
            m_writer.writeCrc();

            return command;
        }

        template <typename T, typename... Ts>
        void write(const T arg, Ts... args)
        {
            m_writer.write(arg);
            write(args...);
        }

        template <typename T, typename... Ts>
        void read(T& arg, Ts... args)
        {
            while (m_reader.nextError())
            {
                uint8_t error = 0;
                m_reader.readError(error);
                // TODO: Throw exception
                printf("Error: 0x%02X\n", error);
            }

            m_reader.read(arg);
            read(args...);
        }
    };
}

#endif // EMBMESSENGER_EMBMESSENGER_HPP