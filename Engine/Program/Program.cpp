module Chord.Engine;

import std;

import Chord.Foundation;

namespace Chord
{
  enum class SerializedNodeType : u8
  {
    Input,
    Output,
    FloatConstant,
    DoubleConstant,
    IntConstant,
    BoolConstant,
    StringConstant,
    Array,
    NativeModuleCall,
    GraphInput,
    GraphOutput,
  };

  static constexpr char Header[] = { 'C', 'H', 'O', 'R', 'D', 'P', 'R', 'O', 'G', 'R', 'A', 'M' };
  static constexpr u32 Version = 0;
  static constexpr u8 HashSalt[] = { 0x8b, 0xe1, 0x53, 0x2f, 0x41, 0x16, 0xc9, 0x8d, 0x1a, 0x2a, 0xb4, 0x3c, 0x0b, 0x34, 0xae, 0xdf };

  template<typename TNode>
  usz GetNodeIndex(const BoundedArray<TNode> &nodes, IProgramGraphNode* node)
  {
    TNode* typedNode = static_cast<TNode*>(node);
    ASSERT(typedNode >= nodes.Elements() && typedNode < nodes.Elements() + nodes.Count());
    return usz(typedNode - nodes.Elements());
  }

  std::optional<Program> Program::Deserialize(Span<const u8> bytes)
  {
    Program program;
    BinaryReader reader(bytes, std::endian::little);

    FixedArray<char, sizeof(Header)> header;
    u32 version;
    FixedArray<u8, Sha256ByteCount> contentHash;
    if (!reader.Read(Span<char>(header))
      || !reader.Read<u32>(&version)
      || !reader.Read(Span<u8>(contentHash)))
      { return std::nullopt; }

    if (std::memcmp(header.Elements(), Header, sizeof(Header)) != 0)
      { return std::nullopt; }

    if (version != Version)
      { return std::nullopt; }

    u32 nativeLibraryDependencyCount;
    if (!reader.Read(&nativeLibraryDependencyCount))
      { return std::nullopt; }

    program.m_nativeLibraryDependencies = InitializeCapacity(nativeLibraryDependencyCount);
    for (u32 i = 0; i < nativeLibraryDependencyCount; i++)
    {
      FixedArray<u8, Guid::ByteCount> idBytes;
      if (!reader.Read(Span<u8>(idBytes))
        || !reader.Read(&program.m_nativeLibraryDependencies[i].m_majorVersion)
        || !reader.Read(&program.m_nativeLibraryDependencies[i].m_minorVersion)
        || !reader.Read(&program.m_nativeLibraryDependencies[i].m_patchVersion))
        { return std::nullopt; }

      program.m_nativeLibraryDependencies[i].m_id = Guid::FromBytes(idBytes);
    }

    if (!reader.Read(&program.m_programVariantProperties.m_sampleRate)
      || !reader.Read(&program.m_programVariantProperties.m_inputChannelCount)
      || !reader.Read(&program.m_programVariantProperties.m_outputChannelCount))
      { return std::nullopt; }

    if (program.m_programVariantProperties.m_sampleRate < 0
      || program.m_programVariantProperties.m_inputChannelCount < 0
      || program.m_programVariantProperties.m_outputChannelCount < 0)
      { return std::nullopt; }

    u32 effectActivationMode;
    if (!reader.Read(&program.m_instrumentProperties.m_maxVoices)
      || !reader.Read(&effectActivationMode)
      || !reader.Read(&program.m_instrumentProperties.m_effectActivationThreshold))
      { return std::nullopt; }

    if (effectActivationMode >= EnumCount<EffectActivationMode>()
      || program.m_instrumentProperties.m_effectActivationThreshold < 0.0)
      { return std::nullopt; }

    program.m_instrumentProperties.m_effectActivationMode = EffectActivationMode(effectActivationMode);

    u32 nodeCount;
    if (!reader.Read(&nodeCount))
      { return std::nullopt; }

    // Read node types and count up the number of each node
    FixedArray<SerializedNodeType> nodeTypes = InitializeCapacity(nodeCount);
    FixedArray<usz, EnumCount<SerializedNodeType>()> nodeCounts;
    nodeCounts.ZeroElements();
    for (u32 nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
      SerializedNodeType nodeType;
      if (!reader.Read(&nodeType))
        { return std::nullopt; }

      if (EnumValue(nodeType) >= EnumCount<SerializedNodeType>())
        { return std::nullopt; }

      nodeTypes[nodeIndex] = nodeType;
      nodeCounts[EnumValue(nodeType)]++;
    }

    program.m_inputNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::Input)]);
    program.m_outputNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::Output)]);
    program.m_floatConstantNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::FloatConstant)]);
    program.m_doubleConstantNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::DoubleConstant)]);
    program.m_intConstantNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::IntConstant)]);
    program.m_boolConstantNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::BoolConstant)]);
    program.m_stringConstantNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::StringConstant)]);
    program.m_arrayNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::Array)]);
    program.m_nativeModuleCallNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::NativeModuleCall)]);
    program.m_graphInputNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::GraphInput)]);
    program.m_graphOutputNodes = InitializeCapacity(nodeCounts[EnumValue(SerializedNodeType::GraphOutput)]);

    // Now, we can go through and pre-resolve all node pointers even though the nodes themselves haven't been allocated because we know where they will live
    FixedArray<IProgramGraphNode*> nodesFromIndices = InitializeCapacity(nodeCount);
    nodeCounts.ZeroElements();
    for (u32 nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
      SerializedNodeType nodeType = nodeTypes[nodeIndex];
      switch (nodeType)
      {
      case SerializedNodeType::Input:
        nodesFromIndices[nodeIndex] = program.m_inputNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::Output:
        nodesFromIndices[nodeIndex] = program.m_outputNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::FloatConstant:
        nodesFromIndices[nodeIndex] = program.m_floatConstantNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::DoubleConstant:
        nodesFromIndices[nodeIndex] = program.m_doubleConstantNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::IntConstant:
        nodesFromIndices[nodeIndex] = program.m_intConstantNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::BoolConstant:
        nodesFromIndices[nodeIndex] = program.m_boolConstantNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::StringConstant:
        nodesFromIndices[nodeIndex] = program.m_stringConstantNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::Array:
        nodesFromIndices[nodeIndex] = program.m_arrayNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::NativeModuleCall:
        nodesFromIndices[nodeIndex] = program.m_nativeModuleCallNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::GraphInput:
        nodesFromIndices[nodeIndex] = program.m_graphInputNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;

      case SerializedNodeType::GraphOutput:
        nodesFromIndices[nodeIndex] = program.m_graphOutputNodes.Elements() + nodeCounts[EnumValue(nodeType)];
        break;
      }

      nodeCounts[EnumValue(nodeTypes[nodeIndex])]++;
    }

    auto AttachProcessorInputNode =
      [&](u32 inputNodeIndex, IProcessorProgramGraphNode* processorNode, const IInputProgramGraphNode** inputNodePointer) -> bool
      {
        if (inputNodeIndex >= nodeCount || nodeTypes[inputNodeIndex] != SerializedNodeType::Input)
          { return false; }

        InputProgramGraphNode* inputNode = static_cast<InputProgramGraphNode*>(nodesFromIndices[inputNodeIndex]);
        if (inputNode->m_processor != nullptr)
          { return false; }

        inputNode->m_processor = processorNode;
        *inputNodePointer = inputNode;
        return true;
      };

    auto AttachProcessorOutputNode =
      [&](u32 outputNodeIndex, IProcessorProgramGraphNode* processorNode, const IOutputProgramGraphNode** outputNodePointer) -> bool
      {
        if (outputNodeIndex >= nodeCount || nodeTypes[outputNodeIndex] != SerializedNodeType::Output)
          { return false; }

        OutputProgramGraphNode* outputNode = static_cast<OutputProgramGraphNode*>(nodesFromIndices[outputNodeIndex]);
        if (outputNode->m_processor != nullptr)
          { return false; }

        outputNode->m_processor = processorNode;
        *outputNodePointer = outputNode;
        return true;
      };

    auto TryGetProcessorNode =
      [&](u32 nodeIndex, const IProcessorProgramGraphNode** processorNodePointer) -> bool
      {
        if (nodeIndex >= nodeCount)
          { return false; }
        switch (nodeTypes[nodeIndex])
        {
        case SerializedNodeType::Input:
          return false;

        case SerializedNodeType::Output:
          return false;

        case SerializedNodeType::FloatConstant:
          *processorNodePointer = static_cast<FloatConstantProgramGraphNode*>(nodesFromIndices[nodeIndex]);
          return true;

        case SerializedNodeType::DoubleConstant:
          *processorNodePointer = static_cast<DoubleConstantProgramGraphNode*>(nodesFromIndices[nodeIndex]);
          return true;

        case SerializedNodeType::IntConstant:
          *processorNodePointer = static_cast<IntConstantProgramGraphNode*>(nodesFromIndices[nodeIndex]);
          return true;

        case SerializedNodeType::BoolConstant:
          *processorNodePointer = static_cast<BoolConstantProgramGraphNode*>(nodesFromIndices[nodeIndex]);
          return true;

        case SerializedNodeType::StringConstant:
          *processorNodePointer = static_cast<StringConstantProgramGraphNode*>(nodesFromIndices[nodeIndex]);
          return true;

        case SerializedNodeType::Array:
          *processorNodePointer = static_cast<ArrayProgramGraphNode*>(nodesFromIndices[nodeIndex]);
          return true;

        case SerializedNodeType::NativeModuleCall:
          *processorNodePointer = static_cast<NativeModuleCallProgramGraphNode*>(nodesFromIndices[nodeIndex]);
          return true;

        case SerializedNodeType::GraphInput:
          *processorNodePointer = static_cast<GraphInputProgramGraphNode*>(nodesFromIndices[nodeIndex]);
          return true;

        case SerializedNodeType::GraphOutput:
          *processorNodePointer = static_cast<GraphOutputProgramGraphNode*>(nodesFromIndices[nodeIndex]);
          return true;

        default:
          ASSERT(false);
          return false;
        }
      };

    // Read node data
    for (u32 nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
    {
      switch (nodeTypes[nodeIndex])
      {
      case SerializedNodeType::Input:
        program.m_inputNodes.AppendNew();
        break;

      case SerializedNodeType::Output:
        {
          u32 connectionCount;
          if (!reader.Read(&connectionCount))
            { return std::nullopt; }

          OutputProgramGraphNode& node = program.m_outputNodes.AppendNew(connectionCount);
          for (u32 i = 0; i < connectionCount; i++)
          {
            u32 connectionNodeIndex;
            if (!reader.Read(&connectionNodeIndex)
              || connectionNodeIndex >= nodeCount
              || nodeTypes[connectionNodeIndex] != SerializedNodeType::Input)
              { return std::nullopt; }
            node.m_connections[i] = static_cast<InputProgramGraphNode*>(nodesFromIndices[connectionNodeIndex]);
          }

          break;
        }

      case SerializedNodeType::FloatConstant:
        {
          u32 outputNodeIndex;
          f32 value;
          if (!reader.Read(&outputNodeIndex) || !reader.Read(&value))
            { return std::nullopt; }

          FloatConstantProgramGraphNode& node = program.m_floatConstantNodes.AppendNew(value);
          if (!AttachProcessorOutputNode(outputNodeIndex, &node, &node.m_output))
            { return std::nullopt; }

          break;
        }

      case SerializedNodeType::DoubleConstant:
        {
          u32 outputNodeIndex;
          f64 value;
          if (!reader.Read(&outputNodeIndex) || !reader.Read(&value))
            { return std::nullopt; }

          DoubleConstantProgramGraphNode& node = program.m_doubleConstantNodes.AppendNew(value);
          if (!AttachProcessorOutputNode(outputNodeIndex, &node, &node.m_output))
            { return std::nullopt; }

          break;
        }

      case SerializedNodeType::IntConstant:
        {
          u32 outputNodeIndex;
          s32 value;
          if (!reader.Read(&outputNodeIndex) || !reader.Read(&value))
            { return std::nullopt; }

          IntConstantProgramGraphNode& node = program.m_intConstantNodes.AppendNew(value);
          if (!AttachProcessorOutputNode(outputNodeIndex, &node, &node.m_output))
            { return std::nullopt; }

          break;
        }

      case SerializedNodeType::BoolConstant:
        {
          u32 outputNodeIndex;
          u8 value;
          if (!reader.Read(&outputNodeIndex) || !reader.Read(&value) || value > 1)
            { return std::nullopt; }

          BoolConstantProgramGraphNode& node = program.m_boolConstantNodes.AppendNew(value != 0);
          if (!AttachProcessorOutputNode(outputNodeIndex, &node, &node.m_output))
            { return std::nullopt; }

          break;
        }

      case SerializedNodeType::StringConstant:
        {
          u32 outputNodeIndex;
          u32 length;
          if (!reader.Read(&outputNodeIndex) || !reader.Read(&length))
            { return std::nullopt; }

          auto [value, buffer] = UnicodeString::CreateForWrite(length);
          if (!reader.Read(buffer))
            { return std::nullopt; }

          StringConstantProgramGraphNode& node = program.m_stringConstantNodes.AppendNew(value);
          if (!AttachProcessorOutputNode(outputNodeIndex, &node, &node.m_output))
            { return std::nullopt; }

          break;
        }

      case SerializedNodeType::Array:
        {
          u32 elementCount;
          if (!reader.Read(&elementCount))
            { return std::nullopt; }

          ArrayProgramGraphNode& node = program.m_arrayNodes.AppendNew(elementCount);
          for (u32 i = 0; i < elementCount; i++)
          {
            u32 elementNodeIndex;
            if (!reader.Read(&elementNodeIndex) || !AttachProcessorInputNode(elementNodeIndex, &node, &node.m_elements[i]))
              { return std::nullopt; }
          }

          u32 outputNodeIndex;
          if (!reader.Read(&outputNodeIndex) || !AttachProcessorOutputNode(outputNodeIndex, &node, &node.m_output))
            { return std::nullopt; }

          break;
        }

      case SerializedNodeType::NativeModuleCall:
        {
          FixedArray<u8, Guid::ByteCount> nativeLibraryIdBytes;
          FixedArray<u8, Guid::ByteCount> nativeModuleIdBytes;
          u32 inputCount;
          u32 outputCount;
          s32 upsampleFactor;
          if (!reader.Read(Span<u8>(nativeLibraryIdBytes))
            || !reader.Read(Span<u8>(nativeModuleIdBytes))
            || !reader.Read(&inputCount)
            || !reader.Read(&outputCount)
            || !reader.Read(&upsampleFactor)
            || upsampleFactor <= 0)
            { return std::nullopt; }

          // Make sure the native library was declared in the dependency list
          Guid nativeLibraryId = Guid::FromBytes(nativeLibraryIdBytes);
          Guid nativeModuleId = Guid::FromBytes(nativeModuleIdBytes);
          bool found = false;
          for (const auto& nativeLibraryDependency : program.m_nativeLibraryDependencies)
          {
            if (nativeLibraryDependency.m_id == nativeLibraryId)
            {
              found = true;
              break;
            }
          }

          if (!found)
            { return std::nullopt; }

          NativeModuleCallProgramGraphNode& node = program.m_nativeModuleCallNodes.AppendNew(
            nativeLibraryId,
            nativeModuleId,
            inputCount,
            outputCount,
            upsampleFactor);

          for (u32 i = 0; i < inputCount; i++)
          {
            u32 inputNodeIndex;
            if (!reader.Read(&inputNodeIndex) || !AttachProcessorInputNode(inputNodeIndex, &node, &node.m_inputs[i]))
              { return std::nullopt; }
          }

          for (u32 i = 0; i < outputCount; i++)
          {
            u32 outputNodeIndex;
            if (!reader.Read(&outputNodeIndex) || !AttachProcessorOutputNode(outputNodeIndex, &node, &node.m_outputs[i]))
              { return std::nullopt; }
          }

          break;
        }

      case SerializedNodeType::GraphInput:
        {
          GraphInputProgramGraphNode& node = program.m_graphInputNodes.AppendNew();

          u32 outputNodeIndex;
          if (!reader.Read(&outputNodeIndex) || !AttachProcessorOutputNode(outputNodeIndex, &node, &node.m_output))
            { return std::nullopt; }

          break;
        }

      case SerializedNodeType::GraphOutput:
        {
          GraphOutputProgramGraphNode& node = program.m_graphOutputNodes.AppendNew();

          u32 inputNodeIndex;
          if (!reader.Read(&inputNodeIndex) || !AttachProcessorInputNode(inputNodeIndex, &node, &node.m_input))
            { return std::nullopt; }

          break;
        }

      default:
        ASSERT(false);
        break;
      }
    }

    // Fill in the input side of connections
    for (const OutputProgramGraphNode& outputNode : program.m_outputNodes)
    {
      if (outputNode.m_processor == nullptr)
        { return std::nullopt; }

      for (const IInputProgramGraphNode* inputNode : outputNode.m_connections)
      {
        InputProgramGraphNode* typedInputNode = static_cast<InputProgramGraphNode*>(const_cast<IInputProgramGraphNode*>(inputNode));
        if (typedInputNode->m_connection != nullptr)
          { return std::nullopt; }
        typedInputNode->m_connection = &outputNode;
      }
    }

    for (const InputProgramGraphNode& inputNode : program.m_inputNodes)
    {
      if (inputNode.m_processor == nullptr || inputNode.m_connection == nullptr)
        { return std::nullopt; }
    }

    u8 hasInputChannelsFloat;
    if (!reader.Read(&hasInputChannelsFloat) || hasInputChannelsFloat > 1)
      { return std::nullopt; }
    if (hasInputChannelsFloat != 0)
    {
      program.m_inputChannelsFloat = InitializeCapacity(usz(program.m_programVariantProperties.m_inputChannelCount));
      program.m_programGraph.m_inputChannelsFloat = program.m_inputChannelsFloat;
      for (const GraphInputProgramGraphNode*& nodePointer : program.m_inputChannelsFloat)
      {
        u32 nodeIndex;
        if (!reader.Read(&nodeIndex) || nodeIndex >= nodeCount || nodeTypes[nodeIndex] != SerializedNodeType::GraphInput)
          { return std::nullopt; }
        nodePointer = static_cast<const GraphInputProgramGraphNode*>(nodesFromIndices[nodeIndex]);
      }
    }

    u8 hasInputChannelsDouble;
    if (!reader.Read(&hasInputChannelsDouble) || hasInputChannelsDouble > 1)
      { return std::nullopt; }
    if (hasInputChannelsDouble != 0)
    {
      program.m_inputChannelsDouble = InitializeCapacity(usz(program.m_programVariantProperties.m_inputChannelCount));
      program.m_programGraph.m_inputChannelsDouble = program.m_inputChannelsDouble;
      for (const GraphInputProgramGraphNode*& nodePointer : program.m_inputChannelsDouble)
      {
        u32 nodeIndex;
        if (!reader.Read(&nodeIndex) || nodeIndex >= nodeCount || nodeTypes[nodeIndex] != SerializedNodeType::GraphInput)
          { return std::nullopt; }
        nodePointer = static_cast<const GraphInputProgramGraphNode*>(nodesFromIndices[nodeIndex]);
      }
    }

    program.m_outputChannels = InitializeCapacity(usz(program.m_programVariantProperties.m_outputChannelCount));
    program.m_programGraph.m_outputChannels = program.m_outputChannels;
    for (const GraphOutputProgramGraphNode*& nodePointer : program.m_outputChannels)
    {
      u32 nodeIndex;
      if (!reader.Read(&nodeIndex) || nodeIndex >= nodeCount || nodeTypes[nodeIndex] != SerializedNodeType::GraphOutput)
        { return std::nullopt; }
      nodePointer = static_cast<const GraphOutputProgramGraphNode*>(nodesFromIndices[nodeIndex]);
    }

    u8 hasVoiceRemainActive;
    if (!reader.Read(&hasVoiceRemainActive) || hasVoiceRemainActive > 1)
      { return std::nullopt; }
    if (hasVoiceRemainActive)
    {
      u32 nodeIndex;
      if (!reader.Read(&nodeIndex) || nodeIndex >= nodeCount || nodeTypes[nodeIndex] != SerializedNodeType::GraphOutput)
        { return std::nullopt; }
      program.m_programGraph.m_voiceRemainActive = static_cast<const GraphOutputProgramGraphNode*>(nodesFromIndices[nodeIndex]);
    }

    u8 hasEffectRemainActive;
    if (!reader.Read(&hasEffectRemainActive) || hasEffectRemainActive > 1)
      { return std::nullopt; }
    if (hasEffectRemainActive)
    {
      u32 nodeIndex;
      if (!reader.Read(&nodeIndex) || nodeIndex >= nodeCount || nodeTypes[nodeIndex] != SerializedNodeType::GraphOutput)
        { return std::nullopt; }
      program.m_programGraph.m_effectRemainActive = static_cast<const GraphOutputProgramGraphNode*>(nodesFromIndices[nodeIndex]);
    }

    u32 voiceToEffectCount;
    if (!reader.Read(&voiceToEffectCount))
      { return std::nullopt; }

    program.m_voiceToEffectPrimitiveTypes = InitializeCapacity(voiceToEffectCount);
    program.m_programGraph.m_voiceToEffectPrimitiveTypes = program.m_voiceToEffectPrimitiveTypes;
    for (PrimitiveType& primitiveType : program.m_voiceToEffectPrimitiveTypes)
    {
      u8 primitiveTypeU8;
      if (!reader.Read<u8>(&primitiveTypeU8))
        { return std::nullopt; }

      if (primitiveType != PrimitiveTypeFloat
        && primitiveType != PrimitiveTypeDouble
        && primitiveType != PrimitiveTypeInt
        && primitiveType != PrimitiveTypeBool)
        { return std::nullopt; }

      primitiveType = PrimitiveType(primitiveTypeU8);
    }

    program.m_voiceToEffectOutputs = InitializeCapacity(voiceToEffectCount);
    program.m_programGraph.m_voiceToEffectOutputs = program.m_voiceToEffectOutputs;
    for (const GraphOutputProgramGraphNode*& nodePointer : program.m_voiceToEffectOutputs)
    {
      u32 nodeIndex;
      if (!reader.Read(&nodeIndex) || nodeIndex >= nodeCount || nodeTypes[nodeIndex] != SerializedNodeType::GraphOutput)
        { return std::nullopt; }
      nodePointer = static_cast<const GraphOutputProgramGraphNode*>(nodesFromIndices[nodeIndex]);
    }

    program.m_voiceToEffectInputs = InitializeCapacity(voiceToEffectCount);
    program.m_programGraph.m_voiceToEffectInputs = program.m_voiceToEffectInputs;
    for (const GraphInputProgramGraphNode*& nodePointer : program.m_voiceToEffectInputs)
    {
      u32 nodeIndex;
      if (!reader.Read(&nodeIndex) || nodeIndex >= nodeCount || nodeTypes[nodeIndex] != SerializedNodeType::GraphInput)
        { return std::nullopt; }
      nodePointer = static_cast<const GraphInputProgramGraphNode*>(nodesFromIndices[nodeIndex]);
    }

    u8 hasVoiceGraph;
    if (!reader.Read(&hasVoiceGraph) || hasVoiceGraph > 1)
      { return std::nullopt; }
    if (hasVoiceGraph)
    {
      u32 voiceGraphCount;
      if (!reader.Read(&voiceGraphCount))
        { return std::nullopt; }
      program.m_voiceGraph = InitializeCapacity(voiceGraphCount);
      program.m_programGraph.m_voiceGraph = program.m_voiceGraph;
      for (const IProcessorProgramGraphNode*& nodePointer : program.m_voiceGraph)
      {
        u32 nodeIndex;
        if (!reader.Read(&nodeIndex) || nodeIndex >= nodeCount || !TryGetProcessorNode(nodeIndex, &nodePointer))
          { return std::nullopt; }
      }
    }

    u8 hasEffectGraph;
    if (!reader.Read(&hasEffectGraph) || hasEffectGraph > 1)
      { return std::nullopt; }
    if (hasEffectGraph)
    {
      u32 effectGraphCount;
      if (!reader.Read(&effectGraphCount))
        { return std::nullopt; }
      program.m_effectGraph = InitializeCapacity(effectGraphCount);
      program.m_programGraph.m_effectGraph = program.m_effectGraph;
      for (const IProcessorProgramGraphNode*& nodePointer : program.m_effectGraph)
      {
        u32 nodeIndex;
        if (!reader.Read(&nodeIndex) || nodeIndex >= nodeCount || !TryGetProcessorNode(nodeIndex, &nodePointer))
          { return std::nullopt; }
      }
    }

    if (reader.GetOffset() != bytes.Count())
      { return std::nullopt; }

    // For simplicity, append the content bytes and the hash salt bytes together so we can do a non-incremental SHA256 hash in a single call
    static constexpr usz HeaderByteCount = sizeof(Header) + sizeof(u32) + Sha256ByteCount;
    usz contentByteCount = bytes.Count() - HeaderByteCount;
    FixedArray<u8> hashInputBytes = InitializeCapacity(contentByteCount + sizeof(HashSalt));
    Span<u8>(hashInputBytes, 0, contentByteCount).CopyElementsFrom(Span(bytes, HeaderByteCount, contentByteCount));
    Span<u8>(hashInputBytes, contentByteCount, sizeof(HashSalt)).CopyElementsFrom(HashSalt);
    auto computedContentHash = CalculateSha256(hashInputBytes);
    if (std::memcmp(contentHash.Elements(), computedContentHash.Elements(), contentHash.Count()) != 0)
      { return std::nullopt; }

    return std::move(program);
  }
}