#include "ScriptValue.h"
#include <sstream>
#include <cmath>

namespace Xi {

    void ScriptValue::SetTable(const std::string& key, const ScriptValue& value) {
        if (m_Type != Type::Table) {
            m_Type = Type::Table;
        }
        m_Table[key] = value;
    }

    ScriptValue ScriptValue::GetTable(const std::string& key) const {
        if (m_Type != Type::Table) return ScriptValue();
        auto it = m_Table.find(key);
        if (it != m_Table.end()) return it->second;
        return ScriptValue();
    }

    bool ScriptValue::HasTable(const std::string& key) const {
        if (m_Type != Type::Table) return false;
        return m_Table.find(key) != m_Table.end();
    }

    ScriptValue ScriptValue::operator+(const ScriptValue& other) const {
        if (m_Type == Type::Number && other.m_Type == Type::Number) {
            return ScriptValue(m_Number + other.m_Number);
        }
        if (m_Type == Type::String || other.m_Type == Type::String) {
            return ScriptValue(ToString() + other.ToString());
        }
        if (m_Type == Type::Vec3 && other.m_Type == Type::Vec3) {
            return ScriptValue(m_Vec3 + other.m_Vec3);
        }
        return ScriptValue();
    }

    ScriptValue ScriptValue::operator-(const ScriptValue& other) const {
        if (m_Type == Type::Number && other.m_Type == Type::Number) {
            return ScriptValue(m_Number - other.m_Number);
        }
        if (m_Type == Type::Vec3 && other.m_Type == Type::Vec3) {
            return ScriptValue(m_Vec3 - other.m_Vec3);
        }
        return ScriptValue();
    }

    ScriptValue ScriptValue::operator*(const ScriptValue& other) const {
        if (m_Type == Type::Number && other.m_Type == Type::Number) {
            return ScriptValue(m_Number * other.m_Number);
        }
        if (m_Type == Type::Vec3 && other.m_Type == Type::Number) {
            return ScriptValue(m_Vec3 * static_cast<float>(other.m_Number));
        }
        if (m_Type == Type::Number && other.m_Type == Type::Vec3) {
            return ScriptValue(other.m_Vec3 * static_cast<float>(m_Number));
        }
        return ScriptValue();
    }

    ScriptValue ScriptValue::operator/(const ScriptValue& other) const {
        if (m_Type == Type::Number && other.m_Type == Type::Number) {
            if (other.m_Number == 0.0) return ScriptValue();
            return ScriptValue(m_Number / other.m_Number);
        }
        if (m_Type == Type::Vec3 && other.m_Type == Type::Number) {
            if (other.m_Number == 0.0) return ScriptValue();
            return ScriptValue(m_Vec3 / static_cast<float>(other.m_Number));
        }
        return ScriptValue();
    }

    ScriptValue ScriptValue::operator-() const {
        if (m_Type == Type::Number) {
            return ScriptValue(-m_Number);
        }
        if (m_Type == Type::Vec3) {
            return ScriptValue(-m_Vec3);
        }
        return ScriptValue();
    }

    bool ScriptValue::operator==(const ScriptValue& other) const {
        if (m_Type != other.m_Type) return false;
        switch (m_Type) {
            case Type::Nil: return true;
            case Type::Bool: return m_Bool == other.m_Bool;
            case Type::Number: return std::abs(m_Number - other.m_Number) < 1e-10;
            case Type::String: return m_String == other.m_String;
            case Type::Vec3: return m_Vec3 == other.m_Vec3;
            default: return false;
        }
    }

    bool ScriptValue::operator!=(const ScriptValue& other) const {
        return !(*this == other);
    }

    bool ScriptValue::operator<(const ScriptValue& other) const {
        if (m_Type == Type::Number && other.m_Type == Type::Number) {
            return m_Number < other.m_Number;
        }
        if (m_Type == Type::String && other.m_Type == Type::String) {
            return m_String < other.m_String;
        }
        return false;
    }

    bool ScriptValue::operator<=(const ScriptValue& other) const {
        return *this < other || *this == other;
    }

    bool ScriptValue::operator>(const ScriptValue& other) const {
        return !(*this <= other);
    }

    bool ScriptValue::operator>=(const ScriptValue& other) const {
        return !(*this < other);
    }

    std::string ScriptValue::ToString() const {
        std::ostringstream ss;
        switch (m_Type) {
            case Type::Nil: return "nil";
            case Type::Bool: return m_Bool ? "true" : "false";
            case Type::Number:
                ss << m_Number;
                return ss.str();
            case Type::String: return m_String;
            case Type::Vec3:
                ss << "Vec3(" << m_Vec3.x << ", " << m_Vec3.y << ", " << m_Vec3.z << ")";
                return ss.str();
            case Type::Table: return "[table]";
            case Type::Function: return "[function]";
            case Type::NativeFunction: return "[native function]";
            case Type::UserData: return "[userdata]";
        }
        return "nil";
    }

}
