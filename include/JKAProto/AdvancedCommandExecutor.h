#pragma once
#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "CommandParser.h"
#include "StringLiteral.h"

namespace JKA::executor {
    // Return value of the command functions. May be useful in the future.
    using CmdRet = int64_t;
    
    // Result of the command execution
    // - executed: were the command executed at all;
    // - command_result: what were returned by the command function,
    // TODO: error reporting (wrong args, no matching overload, etc.)
    struct CommandExecutionResult
    {
        bool executed = false;
        std::optional<CmdRet> command_result{};

        static CommandExecutionResult ok(CmdRet ret)
        {
            return { true, std::move(ret) };
        }

        static CommandExecutionResult fail()
        {
            return { false, {} };
        }
    };

    template<StringLiteral Name, StringLiteral Help>
    struct cmdarg
    {
        constexpr static std::string_view name()
        {
            return Name.value();
        }

        constexpr static std::string_view help()
        {
            return Help.value();
        }
    };

    struct Tail
    {
        std::vector<CommandParser::Argument> value;
    };

    struct TailConcat
    {
        std::string value;
    };

    struct TailConcatEscape
    {
        std::string value;
    };

    // Implementation details
    namespace detail
    {
        using Weight = int64_t;

        enum class ArgKind
        {
            ACTIVE,
            TAG,
        };

        enum class ArgFlags : uint32_t
        {
            NO_FLAGS = 0,

            TAIL = (1 << 0),
            EXCLUSIVE = (1 << 1),
        };
        using ArgFlagsUnderlying = std::underlying_type_t<ArgFlags>;

        constexpr ArgFlags operator|(ArgFlags lhs, ArgFlags rhs)
        {
            return static_cast<ArgFlags>(static_cast<ArgFlagsUnderlying>(lhs) | static_cast<ArgFlagsUnderlying>(rhs));
        }

        constexpr ArgFlags operator&(ArgFlags lhs, ArgFlags rhs)
        {
            return static_cast<ArgFlags>(static_cast<ArgFlagsUnderlying>(lhs) & static_cast<ArgFlagsUnderlying>(rhs));
        }

        constexpr bool flag_on(ArgFlags flags, ArgFlags target_flag)
        {
            return (flags & target_flag) == target_flag;
        }

        // "Active" arg (which would be tied to a command's argument) description
        struct CallbackArgInfoActiveSimple
        {
            CommandParser::ArgType raw_type;
            std::string description{};
            Weight weight{};
        };

        struct CallbackArgInfoActiveTail
        {
        };

        // Tag arg: label tag
        struct CallbackArgInfoTagLabel
        {
            std::string name{};
            std::string help{};
        };

        using CallbackArgInfoVariants = std::variant<
            CallbackArgInfoActiveSimple,
            CallbackArgInfoTagLabel,
            CallbackArgInfoActiveTail
        >;

        struct CallbackArgInfo
        {
            CallbackArgInfoVariants info;
            size_t active_arg_index{};
        };

        struct CallbackArgParsed
        {
            CallbackArgInfoActiveSimple active_info;
            std::optional<CallbackArgInfoTagLabel> tag_label;
        };

        struct CallbackTailArgParsed
        {
            CallbackArgInfoActiveTail info;
            std::optional<CallbackArgInfoTagLabel> tag_label;
        };

        struct CmdArg
        {
            CallbackArgParsed arg_info;
            std::string name{};
            std::string help{};
        };

        // A "metafunction" to convert compile-time
        // argument types to runtime ArgTypeInfo instances.
        // Can be implemented by a simple
        // if constexpr (std::is_same_v<T, int32_t>) { return info_for_int32(); }
        // else if constexpr (std::is_same_v<T, float>) { return info_for_float(); })
        // et cetera.
        // The use of partial specializations is (in some regards) more flexible, so we'll
        // use it for now, although switching to if constexpr chain is a possibility.
        template<typename T>
        struct ArgParser;

        // Partial specializations for each compile-time type we want to support.
        // They all have the same structure, so documentation is provided for ArgParser<int32_t> only.

        template<>
        struct ArgParser<int32_t>
        {
            // Would be used for more precise overload resolution
            static constexpr Weight weight = 2;

            static constexpr ArgKind kind = ArgKind::ACTIVE;
            static constexpr ArgFlags flags = ArgFlags::NO_FLAGS;

            // Converts a runtime Argument into an appropriate value
            static int32_t extract_arg(const CommandParser::Command& cmd, size_t active_arg_idx)
            {
                return cmd.args[active_arg_idx].getInt32();
            }

            // Runtime info regarding this compile-time type.
            static const CallbackArgInfoActiveSimple& get_info()
            {
                return info;
            }

            static inline CallbackArgInfoActiveSimple info = CallbackArgInfoActiveSimple{ CommandParser::ArgType::Int, "int32", weight };
        };

        template<>
        struct ArgParser<int64_t>
        {
            static constexpr Weight weight = 3;

            static constexpr ArgKind kind = ArgKind::ACTIVE;
            static constexpr ArgFlags flags = ArgFlags::NO_FLAGS;

            static int64_t extract_arg(const CommandParser::Command& cmd, size_t active_arg_idx)
            {
                return cmd.args[active_arg_idx].getInt64();
            }

            static const CallbackArgInfoActiveSimple& get_info()
            {
                return info;
            }

            static inline CallbackArgInfoActiveSimple info = CallbackArgInfoActiveSimple{ CommandParser::ArgType::Int, "int64", weight };
        };

        template<>
        struct ArgParser<bool>
        {
            static constexpr int64_t weight = 1;

            static constexpr ArgKind kind = ArgKind::ACTIVE;
            static constexpr ArgFlags flags = ArgFlags::NO_FLAGS;

            static bool extract_arg(const CommandParser::Command& cmd, size_t active_arg_idx)
            {
                return cmd.args[active_arg_idx].getBoolExt();
            }

            static const CallbackArgInfoActiveSimple& get_info()
            {
                return info;
            }

            static inline CallbackArgInfoActiveSimple info = CallbackArgInfoActiveSimple{ CommandParser::ArgType::BoolExt, "bool", weight };
        };

        template<>
        struct ArgParser<float>
        {
            static constexpr Weight weight = 0;

            static constexpr ArgKind kind = ArgKind::ACTIVE;
            static constexpr ArgFlags flags = ArgFlags::NO_FLAGS;

            static float extract_arg(const CommandParser::Command& cmd, size_t active_arg_idx)
            {
                return cmd.args[active_arg_idx].getFloat();
            }

            static const CallbackArgInfoActiveSimple& get_info()
            {
                return info;
            }

            static inline CallbackArgInfoActiveSimple info = CallbackArgInfoActiveSimple{ CommandParser::ArgType::Float, "float", weight };
        };

        template<>
        struct ArgParser<std::string>
        {
            static constexpr Weight weight = 4;

            static constexpr ArgKind kind = ArgKind::ACTIVE;
            static constexpr ArgFlags flags = ArgFlags::NO_FLAGS;

            static const std::string& extract_arg(const CommandParser::Command& cmd, size_t active_arg_idx)
            {
                return cmd.args[active_arg_idx].getStr();
            }

            static const CallbackArgInfoActiveSimple& get_info()
            {
                return info;
            }

            static inline CallbackArgInfoActiveSimple info = CallbackArgInfoActiveSimple{ CommandParser::ArgType::Str, "string", weight };
        };

        template<StringLiteral Name, StringLiteral Help>
        struct ArgParser<cmdarg<Name, Help>>
        {
            static constexpr ArgKind kind = ArgKind::TAG;
            static constexpr ArgFlags flags = ArgFlags::NO_FLAGS;

            constexpr static cmdarg<Name, Help> extract_arg(const CommandParser::Command&, size_t)
            {
                return {};
            }

            static const CallbackArgInfoTagLabel& get_info()
            {
                return info;
            }

            static inline CallbackArgInfoTagLabel info = CallbackArgInfoTagLabel{ std::string(Name.value()), std::string(Help.value()) };
        };

        template<>
        struct ArgParser<Tail>
        {
            static constexpr ArgKind kind = ArgKind::ACTIVE;
            static constexpr ArgFlags flags = ArgFlags::TAIL;

            constexpr static Tail extract_arg(const CommandParser::Command& cmd, size_t active_arg_index)
            {
                std::vector<CommandParser::Argument> res;
                for (size_t i = active_arg_index; i < cmd.args.size(); i++) {
                    res.push_back(cmd.args[i]);
                }
                return Tail{ std::move(res) };
            }

            static const CallbackArgInfoActiveTail& get_info()
            {
                return info;
            }

            static inline CallbackArgInfoActiveTail info = {};
        };

        template<>
        struct ArgParser<TailConcat>
        {
            static constexpr ArgKind kind = ArgKind::ACTIVE;
            static constexpr ArgFlags flags = ArgFlags::TAIL;

            static TailConcat extract_arg(const CommandParser::Command& cmd, size_t active_arg_index)
            {
                return TailConcat{ cmd.concat(active_arg_index) };
            }

            static const CallbackArgInfoActiveTail& get_info()
            {
                return info;
            }

            static inline CallbackArgInfoActiveTail info = {};
        };

        template<>
        struct ArgParser<TailConcatEscape>
        {
            static constexpr ArgKind kind = ArgKind::ACTIVE;
            static constexpr ArgFlags flags = ArgFlags::TAIL;

            static TailConcatEscape extract_arg(const CommandParser::Command& cmd, size_t active_arg_index)
            {
                return TailConcatEscape{ cmd.concatEscape(active_arg_index) };
            }

            static const CallbackArgInfoActiveTail& get_info()
            {
                return info;
            }

            static inline CallbackArgInfoActiveTail info = {};
        };

        struct CompiledCommandArguments
        {
            std::vector<CallbackArgInfo> raw_args;
            std::vector<CallbackArgParsed> args;
            std::optional<CallbackTailArgParsed> tail;
        };

        // A "compiled" command. It is an object which holds all information that is needed to
        // execute a function in runtime: name, runtime arguments info, and, most importanly,
        // a callback object. Callback is an std::function with the same signature for any commands.
        // It is important because we can't make CompiledCommand templated to call various function
        // pointers passed to addCommand() directly, since we must store a set of CompiledCommand in
        // a single map. Thus, the callback is needed: a function which takes a Command
        // and calls the function pointer, passing all arguments it needs.
        struct CompiledCommand
        {
            using Callback = std::function<CmdRet(const CompiledCommandArguments& , const CommandParser::Command&)>;

            std::string name;
            CompiledCommandArguments args;
            Callback callback;

            // If this command's callback could be called with a given Command
            bool matches(const CommandParser::Command& cmd)
            {
                // Too few arguments, never applicable
                if (cmd.args.size() < args.args.size()) {
                    return false;
                }

                // Too many arguments and NO tail
                if (cmd.args.size() > args.args.size() && !args.tail.has_value()) {
                    return false;
                }

                for (size_t i = 0; i < args.args.size(); i++) {
                    if (!cmd.args[i].is(args.args[i].active_info.raw_type)) {
                        return false;  // TODO: report error
                    }
                }

                return true;
            }

            // A helper function to invoke the stored callback
            CmdRet invoke(const CompiledCommandArguments& compiled_args, const CommandParser::Command& cmd)
            {
                return callback(compiled_args, cmd);
            }

            std::ostream& putUsage(std::ostream& os) const
            {
                os << "/" << name;
                for (const auto& arg : args.args) {
                    os << " <";
                    if (arg.tag_label.has_value()) {
                        os << arg.tag_label->name << " : ";
                    }
                    os << arg.active_info.description << ">";
                }
                if (args.tail.has_value()) {
                    if (args.tail->tag_label.has_value()) {
                        os << " [" << args.tail->tag_label->name << "...]";
                    } else {
                        os << " [rest args...]";
                    }
                }
                return os;
            }
        };

        // This function acts as a compiled-time "map()" of sorts:
        // it takes a list of compile-time types (...Args) and constructs
        // a vector of rutime information about each of those args using
        // concrete ArgParser specializations' get_info() function.
        template<typename ...Args>
        std::vector<CallbackArgInfoVariants> parse_args()
        {
            // TODO: see if we may need to *not* remove cvref in some cases
            return { CallbackArgInfoVariants{ ArgParser<std::remove_cvref_t<Args>>::get_info() }... };
        }

        inline std::vector<CallbackArgInfo> enumerate_args(const std::vector<CallbackArgInfoVariants>& args)
        {
            auto res = std::vector<CallbackArgInfo>{};
            size_t index = 0;
            for (const auto& arg : args) {
                res.push_back(CallbackArgInfo{ arg, index });
                if (std::holds_alternative<CallbackArgInfoActiveSimple>(arg)) {
                    index += 1;
                }
            }
            return res;
        }

        template<class... Ts>
        struct overloaded : Ts... { using Ts::operator()...; };

        struct ParsedArgsPack
        {
            std::vector<CallbackArgParsed> args;
            std::optional<CallbackTailArgParsed> tail;
        };

        inline ParsedArgsPack parse_args_pack(const std::vector<CallbackArgInfo>& args)
        {
            std::vector<CallbackArgParsed> res{};
            std::optional<CallbackTailArgParsed> tail{};

            for (const auto& type : args) {
                std::visit(
                    overloaded {
                        [&res, &tail](const CallbackArgInfoActiveSimple& info) {
                            if (tail.has_value()) {
                                throw std::runtime_error("There must be active arguments after a tail one");
                            }
                            res.emplace_back(info);
                        },
                        [&res, &tail](const CallbackArgInfoActiveTail& info) {
                            if (tail.has_value()) {
                                throw std::runtime_error("Only one tail argument must be present");
                            }
                            tail = CallbackTailArgParsed{
                                info,
                                {},
                            };
                        },
                        [&res, &tail](const CallbackArgInfoTagLabel& info) {
                            // Apply the tag to the tail if we have already parsed it
                            if (tail.has_value()) {
                                tail->tag_label = info;
                                return;
                            }

                            if (res.empty()) {
                                throw std::runtime_error("A tag must be applied to an active cmdarg");
                            }

                            res.back().tag_label = info;
                        },
                    },
                    type.info
                );
            }

            return ParsedArgsPack{ std::move(res), std::move(tail) };
        }

        inline std::vector<CallbackArgInfoActiveSimple> get_active_args(const std::vector<CallbackArgInfo>& args)
        {
            std::vector<CallbackArgInfoActiveSimple> res{};
            for (const auto& arg : args) {
                if (std::holds_alternative<CallbackArgInfoActiveSimple>(arg.info)) {
                    res.push_back(std::get<CallbackArgInfoActiveSimple>(arg.info));
                }
            }
            return res;
        }

        // The blackest magic happens here.
        // This function takes a function pointer with arbitrary parameters (within the set of types we support),
        // constructs the runtime information about it, and then constructs a called object, which
        // passes the appropriate runtime arguments into the function.
        template<typename ...Args>
        inline CompiledCommand compile_command_func(std::string name, CmdRet(*func)(Args...))
        {
            std::vector<CallbackArgInfo> arg_types = enumerate_args(parse_args<Args...>());
            ParsedArgsPack args_pack = parse_args_pack(arg_types);

            auto compiled_args = CompiledCommandArguments{
                arg_types,
                args_pack.args,
                args_pack.tail,
            };

            // We need to enumerate our Args... list (parameter pack): that is, for each type in the pack
            // get it's index. For this, we use an index_sequence trick, to get a parameter pack Is with
            // numbers 0, 1, ..., <size of Args...>.
            auto callback = ([func]<size_t ...Is>(std::index_sequence<Is...>)
            {
                // The actual callback.
                return [func](const CompiledCommandArguments& compiled_args, const CommandParser::Command& cmd) {
                    // MSVC workaround: other compilers are able to 
                    // unpack a simple "func(ArgParser<Args>::extract_arg(cmd, cmd.args[Is])...)",
                    // but the MSVC refuses. Because of that we need to directly get a type by index
                    // by using the "tuple trick" - the standard std::get<>() function.
                    //
                    // It works, but significantly slows the compilation, since
                    // std::get<N>(tuple) uses a massive pile of eldritch metaprogramming to actually
                    // get the tuple's Nth type (most probably a recursive parameter pack expansion).
                    using ArgsTuple = std::tuple<Args...>;

                    // Basically, calls func(
                    //     ArgParser<Args[0]>::extract_arg(cmd, cmd.args[0]),
                    //     ArgParser<Args[1]>::extract_arg(cmd, cmd.args[1]),
                    //     ...
                    // ) - this is why we need an extract_arg method.
                    // Also, we could instead pass the cmd and the index to extract_arg, but it does not matter.
                    return func(
                        ArgParser<
                            std::remove_cvref_t<
                                decltype(
                                    std::get<Is>(std::declval<ArgsTuple>())
                                )
                            >
                        >::extract_arg(cmd, compiled_args.raw_args[Is].active_arg_index)...
                    );
                };
            })(std::index_sequence_for<Args...>());

            return CompiledCommand(std::move(name), std::move(compiled_args), std::move(callback));
        }

        template<typename ThisType, typename ...Args>
        inline CompiledCommand compile_command_method(std::string name, ThisType* thisPtr, CmdRet(ThisType::* method)(Args...))
        {
            std::vector<CallbackArgInfo> arg_types = enumerate_args(parse_args<Args...>());
            ParsedArgsPack args_pack = parse_args_pack(arg_types);

            auto compiled_args = CompiledCommandArguments{
                arg_types,
                args_pack.args,
                args_pack.tail,
            };

            auto callback = ([method, thisPtr]<size_t ...Is>(std::index_sequence<Is...>)
            {
                // The actual callback.
                return [method, thisPtr](const CompiledCommandArguments& compiled_args, const CommandParser::Command& cmd) {
                    using ArgsTuple = std::tuple<Args...>;
                    return (thisPtr->*method)(
                        ArgParser<
                            std::remove_cvref_t<
                                decltype(
                                    std::get<Is>(std::declval<ArgsTuple>())
                                )
                            >
                        >::extract_arg(cmd, compiled_args.raw_args[Is].active_arg_index)...
                    );
                };
            })(std::index_sequence_for<Args...>());

            return CompiledCommand(std::move(name), std::move(compiled_args), std::move(callback));
        }

        // Used for overload resolution
        struct CompiledCommandCompare
        {
            constexpr bool operator()(const CompiledCommand& a, const CompiledCommand& b) const noexcept
            {
                // TODO: proper overload sorting
                if (!a.args.tail.has_value() && b.args.tail.has_value()) {
                    return true;  // Overloads with tails should always be the last
                }

                return a.args.args.size() < b.args.args.size();
            }
        };

        // A set of overloads, that is, commands with the same name but different arguments.
        struct OverloadSet
        {
        public:
            OverloadSet() = default;

            // TODO: check for ambiguous overloads
            void add(CompiledCommand cmd)
            {
                overloads.push_back(std::move(cmd));
                // TODO: more efficient approach (heap?)
                std::sort(std::begin(overloads), std::end(overloads), CompiledCommandCompare{});
            }

            // TODO: error reporting, better overload resolution
            CommandExecutionResult invoke(const CommandParser::Command& cmd)
            {
                for (auto& overload : overloads) {
                    if (overload.matches(cmd)) {
                        return CommandExecutionResult::ok(overload.invoke(overload.args, cmd));
                    }
                }
                return CommandExecutionResult::fail();
            }

            std::ostream& putUsage(std::ostream& os) const
            {
                for (const auto& overload : overloads) {
                    overload.putUsage(os);
                    os << std::endl;
                }
                return os;
            }

        private:
            std::vector<CompiledCommand> overloads{};
        };
    }

    class AdvancedCommandExecutor {
    public:
        using Command = CommandParser::Command;
        using Arguments = std::vector<CommandParser::Argument>;
        using Callback = std::function<void(const Command& command)>;

        AdvancedCommandExecutor() = default;
        AdvancedCommandExecutor(const AdvancedCommandExecutor&) = default;
        AdvancedCommandExecutor(AdvancedCommandExecutor&&) noexcept = default;
        AdvancedCommandExecutor& operator=(const AdvancedCommandExecutor&) = default;
        AdvancedCommandExecutor& operator=(AdvancedCommandExecutor&&) noexcept = default;
        ~AdvancedCommandExecutor() = default;

        Command parseCommandString(std::string_view commandString);

        template<typename ...Args>
        void addCommand(std::string_view command, CmdRet(*func)(Args...))
        {
            // Could have been replaced with `overloads[command].add(compile_command_func(...))`
            // if std::map supported passing std::string_view's as keys for operator[].

            auto compiled = detail::compile_command_func(std::string(command), func);

            auto it = overloads.find(command);
            if (it == overloads.end()) {
                auto res = overloads.emplace(std::string(command), detail::OverloadSet());
                res.first->second.add(std::move(compiled));
            } else {
                it->second.add(std::move(compiled));
            }
        }

        template<typename ThisType, typename ...Args>
        void addCommand(std::string_view command, ThisType *thisPtr, CmdRet(ThisType::* method)(Args...))
        {
            auto compiled = detail::compile_command_method(std::string(command), thisPtr, method);

            auto it = overloads.find(command);
            if (it == overloads.end()) {
                auto res = overloads.emplace(std::string(command), detail::OverloadSet());
                res.first->second.add(std::move(compiled));
            } else {
                it->second.add(std::move(compiled));
            }
        }

        CommandExecutionResult execute(const Command & command);

        std::ostream& putUsage(std::ostream& os, std::string_view cmd)
        {
            auto it = overloads.find(cmd);
            if (it == overloads.end()) {
                return os;
            }
            return it->second.putUsage(os);
        }

        std::ostream& putUsageAll(std::ostream& os)
        {
            for (const auto& [cmd_name, cmd_overloads] : overloads) {
                cmd_overloads.putUsage(os);
                os << std::endl;
            }
            return os;
        }

    private:
        std::map<std::string, detail::OverloadSet, std::less<>> overloads{};
    };
}
