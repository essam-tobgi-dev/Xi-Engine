#include "ScriptEngine.h"
#include "../ECS/World.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/RigidBody.h"
#include "../ECS/Components/Light.h"
#include "../Core/Input.h"
#include "../Core/Time.h"
#include "../Core/Log.h"
#include <cmath>
#include <algorithm>
#include <random>

namespace Xi {

    ScriptEngine::ScriptEngine() = default;
    ScriptEngine::~ScriptEngine() = default;

    void ScriptEngine::Init(World* world) {
        m_World = world;
        XI_LOG_INFO("Script Engine initialized");
    }

    void ScriptEngine::Shutdown() {
        m_World = nullptr;
        XI_LOG_INFO("Script Engine shutdown");
    }

    std::unique_ptr<ScriptInterpreter> ScriptEngine::CreateInterpreter() {
        return std::make_unique<ScriptInterpreter>();
    }

    std::vector<StmtNodePtr> ScriptEngine::Compile(const std::string& source) {
        ClearError();

        ScriptLexer lexer(source);
        auto tokens = lexer.Tokenize();

        if (!lexer.GetError().empty()) {
            m_Error = lexer.GetError();
            m_ErrorLine = lexer.GetErrorLine();
            return {};
        }

        ScriptParser parser(tokens);
        auto ast = parser.Parse();

        if (parser.HasError()) {
            m_Error = parser.GetError();
            m_ErrorLine = parser.GetErrorLine();
            return {};
        }

        return ast;
    }

    bool ScriptEngine::Validate(const std::string& source) {
        auto ast = Compile(source);
        return !HasError() && !ast.empty();
    }

    void ScriptEngine::RegisterStandardLibrary(ScriptInterpreter& interp) {
        RegisterMathLibrary(interp);
        RegisterStringLibrary(interp);
        RegisterTableLibrary(interp);

        // print function
        interp.SetGlobal("print", ScriptFunction([](const std::vector<ScriptValue>& args) {
            std::string output;
            for (size_t i = 0; i < args.size(); i++) {
                if (i > 0) output += "\t";
                output += args[i].ToString();
            }
            XI_LOG_INFO("[Script] " + output);
            return ScriptValue();
        }));

        // type function
        interp.SetGlobal("type", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty()) return ScriptValue("nil");
            switch (args[0].GetType()) {
                case ScriptValue::Type::Nil: return ScriptValue("nil");
                case ScriptValue::Type::Bool: return ScriptValue("boolean");
                case ScriptValue::Type::Number: return ScriptValue("number");
                case ScriptValue::Type::String: return ScriptValue("string");
                case ScriptValue::Type::Vec3: return ScriptValue("vec3");
                case ScriptValue::Type::Table: return ScriptValue("table");
                case ScriptValue::Type::Function:
                case ScriptValue::Type::NativeFunction: return ScriptValue("function");
                case ScriptValue::Type::UserData: return ScriptValue("userdata");
            }
            return ScriptValue("unknown");
        }));

        // tonumber
        interp.SetGlobal("tonumber", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty()) return ScriptValue();
            if (args[0].IsNumber()) return args[0];
            if (args[0].IsString()) {
                try {
                    return ScriptValue(std::stod(args[0].AsString()));
                } catch (...) {
                    return ScriptValue();
                }
            }
            return ScriptValue();
        }));

        // tostring
        interp.SetGlobal("tostring", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty()) return ScriptValue("nil");
            return ScriptValue(args[0].ToString());
        }));

        // pairs (simple table iteration)
        interp.SetGlobal("pairs", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsTable()) return ScriptValue();
            return args[0];  // Return the table itself for for-in iteration
        }));

        // ipairs (array iteration)
        interp.SetGlobal("ipairs", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsTable()) return ScriptValue();
            return args[0];
        }));
    }

    void ScriptEngine::RegisterEngineAPI(ScriptInterpreter& interp, Entity entity) {
        RegisterInputAPI(interp);
        RegisterTimeAPI(interp);
        RegisterLogAPI(interp);
        RegisterVec3API(interp);
        RegisterWorldAPI(interp);
        RegisterEntityAPI(interp, entity);
    }

    void ScriptEngine::RegisterMathLibrary(ScriptInterpreter& interp) {
        ScriptValue math = ScriptValue::CreateTable();

        math.SetTable("pi", ScriptValue(3.14159265358979323846));
        math.SetTable("huge", ScriptValue(HUGE_VAL));

        math.SetTable("abs", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::abs(args[0].AsNumber()));
        }));

        math.SetTable("floor", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::floor(args[0].AsNumber()));
        }));

        math.SetTable("ceil", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::ceil(args[0].AsNumber()));
        }));

        math.SetTable("sqrt", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::sqrt(args[0].AsNumber()));
        }));

        math.SetTable("sin", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::sin(args[0].AsNumber()));
        }));

        math.SetTable("cos", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::cos(args[0].AsNumber()));
        }));

        math.SetTable("tan", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::tan(args[0].AsNumber()));
        }));

        math.SetTable("asin", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::asin(args[0].AsNumber()));
        }));

        math.SetTable("acos", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::acos(args[0].AsNumber()));
        }));

        math.SetTable("atan", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(std::atan(args[0].AsNumber()));
        }));

        math.SetTable("atan2", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.size() < 2 || !args[0].IsNumber() || !args[1].IsNumber()) return ScriptValue();
            return ScriptValue(std::atan2(args[0].AsNumber(), args[1].AsNumber()));
        }));

        math.SetTable("rad", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(args[0].AsNumber() * 3.14159265358979323846 / 180.0);
        }));

        math.SetTable("deg", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue();
            return ScriptValue(args[0].AsNumber() * 180.0 / 3.14159265358979323846);
        }));

        math.SetTable("min", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty()) return ScriptValue();
            double result = args[0].IsNumber() ? args[0].AsNumber() : 0;
            for (size_t i = 1; i < args.size(); i++) {
                if (args[i].IsNumber()) {
                    result = std::min(result, args[i].AsNumber());
                }
            }
            return ScriptValue(result);
        }));

        math.SetTable("max", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty()) return ScriptValue();
            double result = args[0].IsNumber() ? args[0].AsNumber() : 0;
            for (size_t i = 1; i < args.size(); i++) {
                if (args[i].IsNumber()) {
                    result = std::max(result, args[i].AsNumber());
                }
            }
            return ScriptValue(result);
        }));

        math.SetTable("clamp", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.size() < 3) return ScriptValue();
            double val = args[0].IsNumber() ? args[0].AsNumber() : 0;
            double minVal = args[1].IsNumber() ? args[1].AsNumber() : 0;
            double maxVal = args[2].IsNumber() ? args[2].AsNumber() : 1;
            return ScriptValue(std::clamp(val, minVal, maxVal));
        }));

        math.SetTable("random", ScriptFunction([](const std::vector<ScriptValue>& args) {
            static std::random_device rd;
            static std::mt19937 gen(rd());

            if (args.empty()) {
                std::uniform_real_distribution<> dis(0.0, 1.0);
                return ScriptValue(dis(gen));
            } else if (args.size() == 1 && args[0].IsNumber()) {
                int max = static_cast<int>(args[0].AsNumber());
                std::uniform_int_distribution<> dis(1, max);
                return ScriptValue(static_cast<double>(dis(gen)));
            } else if (args.size() >= 2 && args[0].IsNumber() && args[1].IsNumber()) {
                int min = static_cast<int>(args[0].AsNumber());
                int max = static_cast<int>(args[1].AsNumber());
                std::uniform_int_distribution<> dis(min, max);
                return ScriptValue(static_cast<double>(dis(gen)));
            }
            return ScriptValue();
        }));

        interp.SetGlobal("math", math);
    }

    void ScriptEngine::RegisterStringLibrary(ScriptInterpreter& interp) {
        ScriptValue str = ScriptValue::CreateTable();

        str.SetTable("len", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsString()) return ScriptValue(0.0);
            return ScriptValue(static_cast<double>(args[0].AsString().size()));
        }));

        str.SetTable("sub", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsString()) return ScriptValue("");
            const std::string& s = args[0].AsString();
            int start = args.size() > 1 && args[1].IsNumber() ? static_cast<int>(args[1].AsNumber()) - 1 : 0;
            int end = args.size() > 2 && args[2].IsNumber() ? static_cast<int>(args[2].AsNumber()) : static_cast<int>(s.size());
            if (start < 0) start = static_cast<int>(s.size()) + start + 1;
            if (end < 0) end = static_cast<int>(s.size()) + end + 1;
            start = std::max(0, start);
            end = std::min(static_cast<int>(s.size()), end);
            if (start >= end) return ScriptValue("");
            return ScriptValue(s.substr(start, end - start));
        }));

        str.SetTable("upper", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsString()) return ScriptValue("");
            std::string s = args[0].AsString();
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            return ScriptValue(s);
        }));

        str.SetTable("lower", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsString()) return ScriptValue("");
            std::string s = args[0].AsString();
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return ScriptValue(s);
        }));

        str.SetTable("format", ScriptFunction([](const std::vector<ScriptValue>& args) {
            // Simple format: just concatenate for now
            std::string result;
            for (const auto& arg : args) {
                result += arg.ToString();
            }
            return ScriptValue(result);
        }));

        interp.SetGlobal("string", str);
    }

    void ScriptEngine::RegisterTableLibrary(ScriptInterpreter& interp) {
        ScriptValue tbl = ScriptValue::CreateTable();

        tbl.SetTable("insert", ScriptFunction([](const std::vector<ScriptValue>& args) {
            // Simplified: table.insert(t, value) appends
            if (args.size() < 2 || !args[0].IsTable()) return ScriptValue();
            auto& tableData = const_cast<std::unordered_map<std::string, ScriptValue>&>(args[0].GetTableData());
            int nextIndex = static_cast<int>(tableData.size()) + 1;
            tableData[std::to_string(nextIndex)] = args[1];
            return ScriptValue();
        }));

        tbl.SetTable("remove", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsTable()) return ScriptValue();
            auto& tableData = const_cast<std::unordered_map<std::string, ScriptValue>&>(args[0].GetTableData());
            if (args.size() > 1 && args[1].IsNumber()) {
                std::string key = std::to_string(static_cast<int>(args[1].AsNumber()));
                auto it = tableData.find(key);
                if (it != tableData.end()) {
                    ScriptValue removed = it->second;
                    tableData.erase(it);
                    return removed;
                }
            }
            return ScriptValue();
        }));

        interp.SetGlobal("table", tbl);
    }

    void ScriptEngine::RegisterInputAPI(ScriptInterpreter& interp) {
        ScriptValue input = ScriptValue::CreateTable();

        input.SetTable("IsKeyDown", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue(false);
            return ScriptValue(Input::IsKeyDown(static_cast<KeyCode>(static_cast<int>(args[0].AsNumber()))));
        }));

        input.SetTable("IsKeyPressed", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue(false);
            return ScriptValue(Input::IsKeyPressed(static_cast<KeyCode>(static_cast<int>(args[0].AsNumber()))));
        }));

        input.SetTable("IsKeyReleased", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue(false);
            return ScriptValue(Input::IsKeyReleased(static_cast<KeyCode>(static_cast<int>(args[0].AsNumber()))));
        }));

        input.SetTable("IsMouseButtonDown", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsNumber()) return ScriptValue(false);
            return ScriptValue(Input::IsMouseButtonDown(static_cast<MouseButton>(static_cast<int>(args[0].AsNumber()))));
        }));

        input.SetTable("GetMousePosition", ScriptFunction([](const std::vector<ScriptValue>&) {
            glm::vec2 pos = Input::GetMousePosition();
            ScriptValue result = ScriptValue::CreateTable();
            result.SetTable("x", ScriptValue(static_cast<double>(pos.x)));
            result.SetTable("y", ScriptValue(static_cast<double>(pos.y)));
            return result;
        }));

        input.SetTable("GetMouseDelta", ScriptFunction([](const std::vector<ScriptValue>&) {
            glm::vec2 delta = Input::GetMouseDelta();
            ScriptValue result = ScriptValue::CreateTable();
            result.SetTable("x", ScriptValue(static_cast<double>(delta.x)));
            result.SetTable("y", ScriptValue(static_cast<double>(delta.y)));
            return result;
        }));

        interp.SetGlobal("Input", input);

        // Key constants
        ScriptValue keys = ScriptValue::CreateTable();
        keys.SetTable("A", ScriptValue(static_cast<double>(KeyCode::A)));
        keys.SetTable("B", ScriptValue(static_cast<double>(KeyCode::B)));
        keys.SetTable("C", ScriptValue(static_cast<double>(KeyCode::C)));
        keys.SetTable("D", ScriptValue(static_cast<double>(KeyCode::D)));
        keys.SetTable("E", ScriptValue(static_cast<double>(KeyCode::E)));
        keys.SetTable("F", ScriptValue(static_cast<double>(KeyCode::F)));
        keys.SetTable("G", ScriptValue(static_cast<double>(KeyCode::G)));
        keys.SetTable("H", ScriptValue(static_cast<double>(KeyCode::H)));
        keys.SetTable("I", ScriptValue(static_cast<double>(KeyCode::I)));
        keys.SetTable("J", ScriptValue(static_cast<double>(KeyCode::J)));
        keys.SetTable("K", ScriptValue(static_cast<double>(KeyCode::K)));
        keys.SetTable("L", ScriptValue(static_cast<double>(KeyCode::L)));
        keys.SetTable("M", ScriptValue(static_cast<double>(KeyCode::M)));
        keys.SetTable("N", ScriptValue(static_cast<double>(KeyCode::N)));
        keys.SetTable("O", ScriptValue(static_cast<double>(KeyCode::O)));
        keys.SetTable("P", ScriptValue(static_cast<double>(KeyCode::P)));
        keys.SetTable("Q", ScriptValue(static_cast<double>(KeyCode::Q)));
        keys.SetTable("R", ScriptValue(static_cast<double>(KeyCode::R)));
        keys.SetTable("S", ScriptValue(static_cast<double>(KeyCode::S)));
        keys.SetTable("T", ScriptValue(static_cast<double>(KeyCode::T)));
        keys.SetTable("U", ScriptValue(static_cast<double>(KeyCode::U)));
        keys.SetTable("V", ScriptValue(static_cast<double>(KeyCode::V)));
        keys.SetTable("W", ScriptValue(static_cast<double>(KeyCode::W)));
        keys.SetTable("X", ScriptValue(static_cast<double>(KeyCode::X)));
        keys.SetTable("Y", ScriptValue(static_cast<double>(KeyCode::Y)));
        keys.SetTable("Z", ScriptValue(static_cast<double>(KeyCode::Z)));
        keys.SetTable("Space", ScriptValue(static_cast<double>(KeyCode::Space)));
        keys.SetTable("Escape", ScriptValue(static_cast<double>(KeyCode::Escape)));
        keys.SetTable("Enter", ScriptValue(static_cast<double>(KeyCode::Enter)));
        keys.SetTable("Tab", ScriptValue(static_cast<double>(KeyCode::Tab)));
        keys.SetTable("LeftShift", ScriptValue(static_cast<double>(KeyCode::LeftShift)));
        keys.SetTable("LeftControl", ScriptValue(static_cast<double>(KeyCode::LeftControl)));
        keys.SetTable("Up", ScriptValue(static_cast<double>(KeyCode::Up)));
        keys.SetTable("Down", ScriptValue(static_cast<double>(KeyCode::Down)));
        keys.SetTable("Left", ScriptValue(static_cast<double>(KeyCode::Left)));
        keys.SetTable("Right", ScriptValue(static_cast<double>(KeyCode::Right)));
        interp.SetGlobal("Key", keys);

        // Mouse button constants
        ScriptValue mouse = ScriptValue::CreateTable();
        mouse.SetTable("Left", ScriptValue(static_cast<double>(MouseButton::Left)));
        mouse.SetTable("Right", ScriptValue(static_cast<double>(MouseButton::Right)));
        mouse.SetTable("Middle", ScriptValue(static_cast<double>(MouseButton::Middle)));
        interp.SetGlobal("Mouse", mouse);
    }

    void ScriptEngine::RegisterTimeAPI(ScriptInterpreter& interp) {
        ScriptValue time = ScriptValue::CreateTable();

        time.SetTable("GetDeltaTime", ScriptFunction([](const std::vector<ScriptValue>&) {
            return ScriptValue(static_cast<double>(Time::GetDeltaTime()));
        }));

        time.SetTable("GetTime", ScriptFunction([](const std::vector<ScriptValue>&) {
            return ScriptValue(static_cast<double>(Time::GetTime()));
        }));

        time.SetTable("GetFPS", ScriptFunction([](const std::vector<ScriptValue>&) {
            return ScriptValue(static_cast<double>(Time::GetFPS()));
        }));

        interp.SetGlobal("Time", time);
    }

    void ScriptEngine::RegisterLogAPI(ScriptInterpreter& interp) {
        ScriptValue log = ScriptValue::CreateTable();

        log.SetTable("Info", ScriptFunction([](const std::vector<ScriptValue>& args) {
            std::string msg;
            for (const auto& arg : args) msg += arg.ToString();
            XI_LOG_INFO("[Script] " + msg);
            return ScriptValue();
        }));

        log.SetTable("Warning", ScriptFunction([](const std::vector<ScriptValue>& args) {
            std::string msg;
            for (const auto& arg : args) msg += arg.ToString();
            XI_LOG_WARN("[Script] " + msg);
            return ScriptValue();
        }));

        log.SetTable("Error", ScriptFunction([](const std::vector<ScriptValue>& args) {
            std::string msg;
            for (const auto& arg : args) msg += arg.ToString();
            XI_LOG_ERROR("[Script] " + msg);
            return ScriptValue();
        }));

        interp.SetGlobal("Log", log);
    }

    void ScriptEngine::RegisterVec3API(ScriptInterpreter& interp) {
        // Vec3 constructor
        interp.SetGlobal("Vec3", ScriptFunction([](const std::vector<ScriptValue>& args) {
            float x = 0, y = 0, z = 0;
            if (args.size() >= 1 && args[0].IsNumber()) x = static_cast<float>(args[0].AsNumber());
            if (args.size() >= 2 && args[1].IsNumber()) y = static_cast<float>(args[1].AsNumber());
            if (args.size() >= 3 && args[2].IsNumber()) z = static_cast<float>(args[2].AsNumber());
            return ScriptValue(glm::vec3(x, y, z));
        }));

        // Vec3 utility functions
        ScriptValue vec3Utils = ScriptValue::CreateTable();

        vec3Utils.SetTable("Length", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsVec3()) return ScriptValue(0.0);
            return ScriptValue(static_cast<double>(glm::length(args[0].AsVec3())));
        }));

        vec3Utils.SetTable("Normalize", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.empty() || !args[0].IsVec3()) return ScriptValue(glm::vec3(0));
            glm::vec3 v = args[0].AsVec3();
            if (glm::length(v) > 0.0001f) {
                return ScriptValue(glm::normalize(v));
            }
            return args[0];
        }));

        vec3Utils.SetTable("Dot", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.size() < 2 || !args[0].IsVec3() || !args[1].IsVec3()) return ScriptValue(0.0);
            return ScriptValue(static_cast<double>(glm::dot(args[0].AsVec3(), args[1].AsVec3())));
        }));

        vec3Utils.SetTable("Cross", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.size() < 2 || !args[0].IsVec3() || !args[1].IsVec3()) return ScriptValue(glm::vec3(0));
            return ScriptValue(glm::cross(args[0].AsVec3(), args[1].AsVec3()));
        }));

        vec3Utils.SetTable("Distance", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.size() < 2 || !args[0].IsVec3() || !args[1].IsVec3()) return ScriptValue(0.0);
            return ScriptValue(static_cast<double>(glm::distance(args[0].AsVec3(), args[1].AsVec3())));
        }));

        vec3Utils.SetTable("Lerp", ScriptFunction([](const std::vector<ScriptValue>& args) {
            if (args.size() < 3 || !args[0].IsVec3() || !args[1].IsVec3() || !args[2].IsNumber()) {
                return ScriptValue(glm::vec3(0));
            }
            float t = static_cast<float>(args[2].AsNumber());
            return ScriptValue(glm::mix(args[0].AsVec3(), args[1].AsVec3(), t));
        }));

        interp.SetGlobal("Vec3Utils", vec3Utils);
    }

    void ScriptEngine::RegisterWorldAPI(ScriptInterpreter& interp) {
        World* world = m_World;

        ScriptValue worldAPI = ScriptValue::CreateTable();

        worldAPI.SetTable("CreateEntity", ScriptFunction([world](const std::vector<ScriptValue>& args) {
            if (!world) return ScriptValue();
            std::string name = args.empty() ? "Entity" : args[0].ToString();
            Entity e = world->CreateEntity(name);
            return ScriptValue(static_cast<double>(e));
        }));

        worldAPI.SetTable("DestroyEntity", ScriptFunction([world](const std::vector<ScriptValue>& args) {
            if (!world || args.empty() || !args[0].IsNumber()) return ScriptValue();
            world->DestroyEntity(static_cast<Entity>(args[0].AsNumber()));
            return ScriptValue();
        }));

        worldAPI.SetTable("GetEntityName", ScriptFunction([world](const std::vector<ScriptValue>& args) {
            if (!world || args.empty() || !args[0].IsNumber()) return ScriptValue("");
            return ScriptValue(world->GetEntityName(static_cast<Entity>(args[0].AsNumber())));
        }));

        worldAPI.SetTable("SetEntityName", ScriptFunction([world](const std::vector<ScriptValue>& args) {
            if (!world || args.size() < 2 || !args[0].IsNumber()) return ScriptValue();
            world->SetEntityName(static_cast<Entity>(args[0].AsNumber()), args[1].ToString());
            return ScriptValue();
        }));

        worldAPI.SetTable("IsEntityValid", ScriptFunction([world](const std::vector<ScriptValue>& args) {
            if (!world || args.empty() || !args[0].IsNumber()) return ScriptValue(false);
            return ScriptValue(world->IsEntityValid(static_cast<Entity>(args[0].AsNumber())));
        }));

        interp.SetGlobal("World", worldAPI);
    }

    void ScriptEngine::RegisterEntityAPI(ScriptInterpreter& interp, Entity entity) {
        World* world = m_World;

        // Set current entity
        interp.SetGlobal("entity", ScriptValue(static_cast<double>(entity)));

        // GetTransform - returns table with position, rotation, scale
        interp.SetGlobal("GetTransform", ScriptFunction([world, entity](const std::vector<ScriptValue>&) {
            if (!world || !world->HasComponent<Transform>(entity)) return ScriptValue();

            Transform& t = world->GetComponent<Transform>(entity);
            ScriptValue result = ScriptValue::CreateTable();
            result.SetTable("position", ScriptValue(t.position));
            result.SetTable("rotation", ScriptValue(t.rotation));
            result.SetTable("scale", ScriptValue(t.scale));
            return result;
        }));

        // SetPosition
        interp.SetGlobal("SetPosition", ScriptFunction([world, entity](const std::vector<ScriptValue>& args) {
            if (!world || !world->HasComponent<Transform>(entity)) return ScriptValue();
            if (args.empty()) return ScriptValue();

            Transform& t = world->GetComponent<Transform>(entity);
            if (args[0].IsVec3()) {
                t.position = args[0].AsVec3();
            } else if (args.size() >= 3) {
                t.position.x = args[0].IsNumber() ? static_cast<float>(args[0].AsNumber()) : t.position.x;
                t.position.y = args[1].IsNumber() ? static_cast<float>(args[1].AsNumber()) : t.position.y;
                t.position.z = args[2].IsNumber() ? static_cast<float>(args[2].AsNumber()) : t.position.z;
            }
            return ScriptValue();
        }));

        // SetRotation
        interp.SetGlobal("SetRotation", ScriptFunction([world, entity](const std::vector<ScriptValue>& args) {
            if (!world || !world->HasComponent<Transform>(entity)) return ScriptValue();
            if (args.empty()) return ScriptValue();

            Transform& t = world->GetComponent<Transform>(entity);
            if (args[0].IsVec3()) {
                t.rotation = args[0].AsVec3();
            } else if (args.size() >= 3) {
                t.rotation.x = args[0].IsNumber() ? static_cast<float>(args[0].AsNumber()) : t.rotation.x;
                t.rotation.y = args[1].IsNumber() ? static_cast<float>(args[1].AsNumber()) : t.rotation.y;
                t.rotation.z = args[2].IsNumber() ? static_cast<float>(args[2].AsNumber()) : t.rotation.z;
            }
            return ScriptValue();
        }));

        // SetScale
        interp.SetGlobal("SetScale", ScriptFunction([world, entity](const std::vector<ScriptValue>& args) {
            if (!world || !world->HasComponent<Transform>(entity)) return ScriptValue();
            if (args.empty()) return ScriptValue();

            Transform& t = world->GetComponent<Transform>(entity);
            if (args[0].IsVec3()) {
                t.scale = args[0].AsVec3();
            } else if (args.size() >= 3) {
                t.scale.x = args[0].IsNumber() ? static_cast<float>(args[0].AsNumber()) : t.scale.x;
                t.scale.y = args[1].IsNumber() ? static_cast<float>(args[1].AsNumber()) : t.scale.y;
                t.scale.z = args[2].IsNumber() ? static_cast<float>(args[2].AsNumber()) : t.scale.z;
            } else if (args[0].IsNumber()) {
                float s = static_cast<float>(args[0].AsNumber());
                t.scale = glm::vec3(s);
            }
            return ScriptValue();
        }));

        // Translate (move by delta)
        interp.SetGlobal("Translate", ScriptFunction([world, entity](const std::vector<ScriptValue>& args) {
            if (!world || !world->HasComponent<Transform>(entity)) return ScriptValue();
            if (args.empty()) return ScriptValue();

            Transform& t = world->GetComponent<Transform>(entity);
            if (args[0].IsVec3()) {
                t.position += args[0].AsVec3();
            } else if (args.size() >= 3) {
                if (args[0].IsNumber()) t.position.x += static_cast<float>(args[0].AsNumber());
                if (args[1].IsNumber()) t.position.y += static_cast<float>(args[1].AsNumber());
                if (args[2].IsNumber()) t.position.z += static_cast<float>(args[2].AsNumber());
            }
            return ScriptValue();
        }));

        // Rotate (rotate by delta angles)
        interp.SetGlobal("Rotate", ScriptFunction([world, entity](const std::vector<ScriptValue>& args) {
            if (!world || !world->HasComponent<Transform>(entity)) return ScriptValue();
            if (args.empty()) return ScriptValue();

            Transform& t = world->GetComponent<Transform>(entity);
            if (args[0].IsVec3()) {
                t.rotation += args[0].AsVec3();
            } else if (args.size() >= 3) {
                if (args[0].IsNumber()) t.rotation.x += static_cast<float>(args[0].AsNumber());
                if (args[1].IsNumber()) t.rotation.y += static_cast<float>(args[1].AsNumber());
                if (args[2].IsNumber()) t.rotation.z += static_cast<float>(args[2].AsNumber());
            }
            return ScriptValue();
        }));

        // GetForward
        interp.SetGlobal("GetForward", ScriptFunction([world, entity](const std::vector<ScriptValue>&) {
            if (!world || !world->HasComponent<Transform>(entity)) return ScriptValue(glm::vec3(0, 0, -1));
            Transform& t = world->GetComponent<Transform>(entity);
            return ScriptValue(t.GetForward());
        }));

        // GetRight
        interp.SetGlobal("GetRight", ScriptFunction([world, entity](const std::vector<ScriptValue>&) {
            if (!world || !world->HasComponent<Transform>(entity)) return ScriptValue(glm::vec3(1, 0, 0));
            Transform& t = world->GetComponent<Transform>(entity);
            return ScriptValue(t.GetRight());
        }));

        // GetUp
        interp.SetGlobal("GetUp", ScriptFunction([world, entity](const std::vector<ScriptValue>&) {
            if (!world || !world->HasComponent<Transform>(entity)) return ScriptValue(glm::vec3(0, 1, 0));
            Transform& t = world->GetComponent<Transform>(entity);
            return ScriptValue(t.GetUp());
        }));
    }

}
