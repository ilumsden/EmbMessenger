#ifndef EMBMESSENGER_EMBMESSENGER_HPP
#define EMBMESSENGER_EMBMESSENGER_HPP

#include <setjmp.h>
#include <stdint.h>

#include "EmbMessenger/DataError.hpp"
#include "EmbMessenger/Reader.hpp"
#include "EmbMessenger/Writer.hpp"
#include "Templates.hpp"

#ifdef EMB_TESTING
#include <functional>
#endif

namespace emb
{
    namespace shared
    {
        class IBuffer;
    }

    namespace device
    {
        template <uint8_t MaxCommands, uint8_t MaxPeriodicCommands>
        class EmbMessenger
        {
        protected:
#ifndef EMB_TESTING
            using CommandFunction = void (*)(void);
            using TimeFunction = uint32_t (*)(void);
#else
            using CommandFunction = std::function<void()>;
            using TimeFunction = std::function<uint32_t()>;
#endif

            struct PeriodicCommand
            {
                uint8_t command_id = 255;
                uint16_t message_id = 0;
                uint32_t time_interval = 0;
                uint32_t next_time = 0;
            };

            shared::IBuffer* m_buffer;
            shared::Reader m_reader;
            shared::Writer m_writer;
            jmp_buf m_jmp_buf;

            uint8_t m_command_id;
            uint16_t m_message_id;
            bool m_is_periodic;
            uint8_t m_num_messages;

            uint16_t m_parameter_index;

            TimeFunction m_time_func;

            CommandFunction m_commands[MaxCommands];
            PeriodicCommand m_periodic_commands[MaxPeriodicCommands];

            template <typename T, typename... Ts>
            void read_helper(false_type, T& value, Ts&&... args)
            {
                read(value);
                read(args...);
            }

            template <typename T, typename F, typename... Ts>
            void read_helper(true_type, T& value, F&& validate)
            {
                read(value);

                if (!validate(value))
                {
                    reportError(static_cast<shared::DataError>(shared::DataError::kParameterInvalid),
                                m_parameter_index - 1u);
                }

                checkCrc();
            }

            template <typename T, typename F, typename... Ts>
            void read_helper(true_type, T& value, F&& validate, Ts&&... args)
            {
                read(value);

                if (!validate(value))
                {
                    reportError(static_cast<shared::DataError>(shared::DataError::kParameterInvalid),
                                m_parameter_index - 1u);
                }

                read(args...);
            }

            void write()
            {
                (void)0;
            }

            void consumeMessage()
            {
                for (uint8_t n = m_buffer->messages();
                     n > 0 && n == m_buffer->messages() && n == m_num_messages && !m_buffer->empty();
                     m_buffer->readByte())
                    ;
            }

            void resetPeriodicCommands()
            {
                checkCrc();
                for (uint8_t i = 0; i < MaxPeriodicCommands; ++i)
                {
                    m_periodic_commands[i].command_id = 255;
                    m_periodic_commands[i].message_id = 0;
                    m_periodic_commands[i].time_interval = 0;
                    m_periodic_commands[i].next_time = 0;
                }
            }

            void registerPeriodicCommand()
            {
                uint8_t periodic_command_id;
                uint32_t periodic_interval;

                read(periodic_command_id, periodic_interval);

                if (m_commands[periodic_command_id] == nullptr)
                {
                    reportError(shared::DataError::kParameterInvalid, 0);
                }

                bool registeredCommand = false;
                for (uint8_t i = 0; i < MaxPeriodicCommands; ++i)
                {
                    if (m_periodic_commands[i].command_id >= 0xF0)
                    {
                        m_periodic_commands[i].command_id = periodic_command_id;
                        m_periodic_commands[i].message_id = m_message_id;
                        m_periodic_commands[i].time_interval = periodic_interval;
                        m_periodic_commands[i].next_time = m_time_func();
                        registeredCommand = true;
                        break;
                    }
                }

                if (!registeredCommand)
                {
                    reportError(shared::DataError::kOutOfPeriodicCommandSlots);
                    return;
                }
            }

            void unregisterPeriodicCommand()
            {
                uint8_t periodic_command_id;

                read(periodic_command_id);

                for (uint8_t i = 0; i < MaxPeriodicCommands; ++i)
                {
                    if (m_periodic_commands[i].command_id == periodic_command_id)
                    {
                        write(m_periodic_commands[i].message_id);

                        m_periodic_commands[i].command_id = 255;
                        m_periodic_commands[i].message_id = 0;
                        m_periodic_commands[i].time_interval = 0;
                        m_periodic_commands[i].next_time = 0;
                        return;
                    }
                }

                reportError(shared::DataError::kParameterInvalid, 0);
            }

        public:
            EmbMessenger(shared::IBuffer* buffer) :
                m_buffer(buffer),
                m_reader(buffer),
                m_writer(buffer)
            {
                static_assert(MaxCommands < 0xF0, "MaxCommands must be less than 0xF0 (240)");
                static_assert(MaxPeriodicCommands == 0, "TimeFunction required for periodic commands");

                for (uint8_t i = 0; i < MaxCommands; ++i)
                {
                    m_commands[i] = nullptr;
                }
            }

            EmbMessenger(shared::IBuffer* buffer, TimeFunction timeFunc) :
                m_buffer(buffer),
                m_reader(buffer),
                m_writer(buffer),
                m_time_func(timeFunc)
            {
                static_assert(MaxCommands < 0xF0, "MaxCommands must be less than 0xF0 (240)");
                static_assert(MaxPeriodicCommands < 0xF0, "MaxPeriodicCommands must be less than 0xF0 (240)");

                for (uint8_t i = 0; i < MaxCommands; ++i)
                {
                    m_commands[i] = nullptr;
                }
            }

            bool registerCommand(int id, CommandFunction command)
            {
                if (id >= MaxCommands)
                {
                    return false;
                }

                if (m_commands[id] != nullptr)
                {
                    return false;
                }

                m_commands[id] = command;
                return true;
            }

            uint8_t getCommandId() const
            {
                return m_command_id;
            }

            uint16_t getMessageId() const
            {
                return m_message_id;
            }

            bool getIsPeriodic() const
            {
                return m_is_periodic;
            }

            void update()
            {
                m_buffer->update();
                m_num_messages = m_buffer->messages();

                if (m_num_messages != 0)
                {
                    m_reader.resetCrc();
                    m_message_id = 0;
                    m_parameter_index = 0;

                    if (!m_reader.read(m_message_id))
                    {
                        m_writer.writeError(shared::DataError::kMessageIdReadError);
                        m_writer.write(0);
                        consumeMessage();
                        m_writer.writeCrc();
                        return;
                    }
                    m_writer.write(m_message_id);

                    if (!m_reader.read(m_command_id))
                    {
                        m_writer.writeError(shared::DataError::kCommandIdReadError);
                        m_writer.write(0);
                        consumeMessage();
                        m_writer.writeCrc();
                        return;
                    }

                    if (setjmp(m_jmp_buf) == 0)
                    {
                        switch (m_command_id)
                        {
                            case 0xFF:
                                resetPeriodicCommands();
                                break;
                            case 0xFE:
                                registerPeriodicCommand();
                                break;
                            case 0xFD:
                                unregisterPeriodicCommand();
                                break;
                            default:
                                if (m_command_id >= MaxCommands || m_commands[m_command_id] == nullptr)
                                {
                                    m_writer.writeError(shared::DataError::kCommandIdInvalid);
                                    m_writer.write(m_command_id);
                                    consumeMessage();
                                }
                                else
                                {
                                    m_commands[m_command_id]();
                                }
                        }
                    }
                    else
                    {
                        consumeMessage();
                    }

                    if (m_buffer->messages() == m_num_messages)
                    {
                        if (m_reader.nextCrc())
                        {
                            if (!m_reader.readCrc())
                            {
                                consumeMessage();
                                m_writer.writeError(shared::DataError::kCrcInvalid);
                                m_writer.write(0);
                            }
                        }
                        else
                        {
                            consumeMessage();
                            m_writer.writeError(shared::DataError::kExtraParameters);
                            m_writer.write(0);
                        }
                    }

                    m_writer.writeCrc();
                }

                if (MaxPeriodicCommands > 0)
                {
                    m_is_periodic = true;
                    uint32_t current_time = m_time_func();
                    for (uint8_t i = 0; i < MaxPeriodicCommands; ++i)
                    {
                        if (m_periodic_commands[i].command_id < 0xF0 && m_periodic_commands[i].next_time <= current_time)
                        {
                            m_command_id = m_periodic_commands[i].command_id;
                            m_message_id = m_periodic_commands[i].message_id;
                            m_parameter_index = 0;

                            m_writer.write(m_message_id);
                            if (setjmp(m_jmp_buf) == 0)
                            {
                                m_commands[m_command_id]();
                            }
                            m_writer.writeCrc();
                            m_periodic_commands[i].next_time += m_periodic_commands[i].time_interval;
                            current_time = m_time_func();
                        }
                    }
                }
            }

            void checkCrc()
            {
                if (m_reader.nextCrc() && !m_reader.readCrc())
                {
                    reportError(shared::DataError::kCrcInvalid);
                }
            }

            template <typename T>
            void read(T& value)
            {
                if (!m_reader.read(value))
                {
                    reportError(static_cast<shared::DataError>(shared::DataError::kParameterReadError),
                                m_parameter_index);
                }

                ++m_parameter_index;

                checkCrc();
            }

            template <typename T, typename F, typename... Ts>
            void read(T& value, F&& validator, Ts&&... args)
            {
                read_helper(is_validator_t<T, F>{}, value, validator, args...);
            }

            template <typename T, typename... Ts>
            void write(const T value, const Ts... args)
            {
                m_writer.write(value);

                write(args...);
            }

            void reportError(const shared::DataError code, const int16_t data = 0)
            {
                m_writer.writeError(code);
                m_writer.write(data);

                longjmp(m_jmp_buf, code);
            }
        };
    }  // namespace device
}  // namespace emb

#endif  // EMBMESSENGER_EMBMESSENGER_HPP
