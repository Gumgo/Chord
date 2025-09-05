export module Chord.Engine:Program.ProgramGraphNodes.ConstantProgramGraphNode;

import std;

import Chord.Foundation;
import :Program.ProgramGraphNodes.IProcessorProgramGraphNode;
import :Program.ProgramGraphNodes.IProgramGraphNode;

namespace Chord
{
  export
  {
    class IOutputProgramGraphNode;

    class FloatConstantProgramGraphNode : public IProgramGraphNode, public IProcessorProgramGraphNode
    {
    public:
      FloatConstantProgramGraphNode(f32 value)
        : m_value(value)
        { }

      FloatConstantProgramGraphNode(const FloatConstantProgramGraphNode&) = delete;
      FloatConstantProgramGraphNode& operator=(const FloatConstantProgramGraphNode&) = delete;

      FloatConstantProgramGraphNode(FloatConstantProgramGraphNode&& other) noexcept
        : m_value(std::exchange(other.m_value, 0.0f))
        , m_output(std::exchange(other.m_output, nullptr))
        { }

      FloatConstantProgramGraphNode& operator=(FloatConstantProgramGraphNode&& other) noexcept
      {
        m_value = std::exchange(other.m_value, 0.0f);
        m_output = std::exchange(other.m_output, nullptr);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::FloatConstant; }

      f32 Value() const
        { return m_value; }

    private:
      friend class Program;

      f32 m_value = 0.0f;
      const IOutputProgramGraphNode* m_output = nullptr;
    };

    class DoubleConstantProgramGraphNode : public IProgramGraphNode, public IProcessorProgramGraphNode
    {
    public:
      DoubleConstantProgramGraphNode(f64 value)
        : m_value(value)
        { }

      DoubleConstantProgramGraphNode(const DoubleConstantProgramGraphNode&) = delete;
      DoubleConstantProgramGraphNode& operator=(const DoubleConstantProgramGraphNode&) = delete;

      DoubleConstantProgramGraphNode(DoubleConstantProgramGraphNode&& other) noexcept
        : m_value(std::exchange(other.m_value, 0.0))
        , m_output(std::exchange(other.m_output, nullptr))
        { }

      DoubleConstantProgramGraphNode& operator=(DoubleConstantProgramGraphNode&& other) noexcept
      {
        m_value = std::exchange(other.m_value, 0.0);
        m_output = std::exchange(other.m_output, nullptr);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::DoubleConstant; }

      f64 Value() const
        { return m_value; }

    private:
      friend class Program;

      f64 m_value = 0.0;
      const IOutputProgramGraphNode* m_output = nullptr;
    };

    class IntConstantProgramGraphNode : public IProgramGraphNode, public IProcessorProgramGraphNode
    {
    public:
      IntConstantProgramGraphNode(s32 value)
        : m_value(value)
        { }

      IntConstantProgramGraphNode(const IntConstantProgramGraphNode&) = delete;
      IntConstantProgramGraphNode& operator=(const IntConstantProgramGraphNode&) = delete;

      IntConstantProgramGraphNode(IntConstantProgramGraphNode&& other) noexcept
        : m_value(std::exchange(other.m_value, 0))
        , m_output(std::exchange(other.m_output, nullptr))
        { }

      IntConstantProgramGraphNode& operator=(IntConstantProgramGraphNode&& other) noexcept
      {
        m_value = std::exchange(other.m_value, 0);
        m_output = std::exchange(other.m_output, nullptr);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::IntConstant; }

      s32 Value() const
        { return m_value; }

    private:
      friend class Program;

      s32 m_value = 0;
      const IOutputProgramGraphNode* m_output = nullptr;
    };

    class BoolConstantProgramGraphNode : public IProgramGraphNode, public IProcessorProgramGraphNode
    {
    public:
      BoolConstantProgramGraphNode(bool value)
        : m_value(value)
        { }

      BoolConstantProgramGraphNode(const BoolConstantProgramGraphNode&) = delete;
      BoolConstantProgramGraphNode& operator=(const BoolConstantProgramGraphNode&) = delete;

      BoolConstantProgramGraphNode(BoolConstantProgramGraphNode&& other) noexcept
        : m_value(std::exchange(other.m_value, false))
        , m_output(std::exchange(other.m_output, nullptr))
        { }

      BoolConstantProgramGraphNode& operator=(BoolConstantProgramGraphNode&& other) noexcept
      {
        m_value = std::exchange(other.m_value, false);
        m_output = std::exchange(other.m_output, nullptr);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::BoolConstant; }

      bool Value() const
        { return m_value; }

    private:
      friend class Program;

      bool m_value = false;
      const IOutputProgramGraphNode* m_output = nullptr;
    };

    class StringConstantProgramGraphNode : public IProgramGraphNode, public IProcessorProgramGraphNode
    {
    public:
      StringConstantProgramGraphNode(UnicodeString value)
        : m_value(value)
        { }

      StringConstantProgramGraphNode(const StringConstantProgramGraphNode&) = delete;
      StringConstantProgramGraphNode& operator=(const StringConstantProgramGraphNode&) = delete;

      StringConstantProgramGraphNode(StringConstantProgramGraphNode&& other) noexcept
        : m_value(std::exchange(other.m_value, {}))
        , m_output(std::exchange(other.m_output, nullptr))
        { }

      StringConstantProgramGraphNode& operator=(StringConstantProgramGraphNode&& other) noexcept
      {
        m_value = std::exchange(other.m_value, {});
        m_output = std::exchange(other.m_output, nullptr);
        return *this;
      }

      virtual ProgramGraphNodeType Type() const override
        { return ProgramGraphNodeType::StringConstant; }

      const UnicodeString &Value() const
        { return m_value; }

    private:
      friend class Program;

      UnicodeString m_value;
      const IOutputProgramGraphNode* m_output = nullptr;
    };
  }
}