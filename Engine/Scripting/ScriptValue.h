#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <variant>
#include <glm/glm.hpp>

namespace Xi {

    class ScriptValue;
    using ScriptFunction = std::function<ScriptValue(const std::vector<ScriptValue>&)>;

    // Represents a value in the scripting system
    class ScriptValue {
    public:
        enum class Type {
            Nil,
            Bool,
            Number,
            String,
            Vec3,
            Table,
            Function,
            NativeFunction,
            UserData
        };

        ScriptValue() : m_Type(Type::Nil) {}
        ScriptValue(bool b) : m_Type(Type::Bool), m_Bool(b) {}
        ScriptValue(double n) : m_Type(Type::Number), m_Number(n) {}
        ScriptValue(float n) : m_Type(Type::Number), m_Number(static_cast<double>(n)) {}
        ScriptValue(int n) : m_Type(Type::Number), m_Number(static_cast<double>(n)) {}
        ScriptValue(const std::string& s) : m_Type(Type::String), m_String(s) {}
        ScriptValue(const char* s) : m_Type(Type::String), m_String(s) {}
        ScriptValue(const glm::vec3& v) : m_Type(Type::Vec3), m_Vec3(v) {}
        ScriptValue(ScriptFunction func) : m_Type(Type::NativeFunction), m_NativeFunc(func) {}
        ScriptValue(void* ptr) : m_Type(Type::UserData), m_UserData(ptr) {}

        Type GetType() const { return m_Type; }

        bool IsNil() const { return m_Type == Type::Nil; }
        bool IsBool() const { return m_Type == Type::Bool; }
        bool IsNumber() const { return m_Type == Type::Number; }
        bool IsString() const { return m_Type == Type::String; }
        bool IsVec3() const { return m_Type == Type::Vec3; }
        bool IsTable() const { return m_Type == Type::Table; }
        bool IsFunction() const { return m_Type == Type::Function || m_Type == Type::NativeFunction; }
        bool IsUserData() const { return m_Type == Type::UserData; }

        bool IsTrue() const {
            if (m_Type == Type::Nil) return false;
            if (m_Type == Type::Bool) return m_Bool;
            return true;
        }

        bool AsBool() const { return m_Bool; }
        double AsNumber() const { return m_Number; }
        const std::string& AsString() const { return m_String; }
        const glm::vec3& AsVec3() const { return m_Vec3; }
        glm::vec3& AsVec3() { return m_Vec3; }
        void* AsUserData() const { return m_UserData; }
        const ScriptFunction& AsNativeFunction() const { return m_NativeFunc; }

        // Table operations
        void SetTable(const std::string& key, const ScriptValue& value);
        ScriptValue GetTable(const std::string& key) const;
        bool HasTable(const std::string& key) const;
        std::unordered_map<std::string, ScriptValue>& GetTableData() { return m_Table; }
        const std::unordered_map<std::string, ScriptValue>& GetTableData() const { return m_Table; }

        // Stored function (bytecode index)
        void SetFunctionIndex(int index) { m_Type = Type::Function; m_FuncIndex = index; }
        int GetFunctionIndex() const { return m_FuncIndex; }

        // Operators
        ScriptValue operator+(const ScriptValue& other) const;
        ScriptValue operator-(const ScriptValue& other) const;
        ScriptValue operator*(const ScriptValue& other) const;
        ScriptValue operator/(const ScriptValue& other) const;
        ScriptValue operator-() const;
        bool operator==(const ScriptValue& other) const;
        bool operator!=(const ScriptValue& other) const;
        bool operator<(const ScriptValue& other) const;
        bool operator<=(const ScriptValue& other) const;
        bool operator>(const ScriptValue& other) const;
        bool operator>=(const ScriptValue& other) const;

        std::string ToString() const;

        static ScriptValue CreateTable() {
            ScriptValue v;
            v.m_Type = Type::Table;
            return v;
        }

    private:
        Type m_Type = Type::Nil;
        bool m_Bool = false;
        double m_Number = 0.0;
        std::string m_String;
        glm::vec3 m_Vec3 = glm::vec3(0.0f);
        std::unordered_map<std::string, ScriptValue> m_Table;
        ScriptFunction m_NativeFunc;
        void* m_UserData = nullptr;
        int m_FuncIndex = -1;
    };

}
