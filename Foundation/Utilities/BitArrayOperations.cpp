module Chord.Foundation;

import std;

namespace Chord
{
  // !!! write thorough unit tests
  constexpr void CopyBits(Span<u8> destination, usz destinationOffset, Span<const u8> source, usz sourceOffset, usz count)
  {
    ASSERT(destinationOffset + count <= destination.Count() * 8);
    ASSERT(sourceOffset + count <= source.Count() * 8);

    if (count == 0)
      { return; }

    usz sourceByteIndex = sourceOffset / 8;
    usz sourceByteOffset = sourceOffset % 8;
    usz endSourceByteIndex = (sourceOffset + count + 7) / 8;
    usz destinationByteIndex = destinationOffset / 8;
    usz destinationByteOffset = destinationOffset % 8;
    usz endDestinationByteIndex = (destinationOffset + count + 7) / 8;

    if (count < 7)
    {
      // We're copying fewer than 7 bits which means we'll need to potentially chop off both sides. Handle this as a special case.
      u8 unmaskedSourceByte = u8(source[sourceByteIndex] << sourceByteOffset);
      if ((sourceOffset + count - 1) / 8 > sourceByteIndex)
      {
        // The source spans two bytes
        unmaskedSourceByte &= u8(~(~0 << sourceByteOffset));
        unmaskedSourceByte |= u8(source[sourceByteIndex + 1] >> (8 - sourceByteOffset));
      }

      u16 destinationMask = u8(~(~0 << count) << destinationByteOffset);
      u16 destinationBytes = u16((unmaskedSourceByte << destinationByteOffset) & destinationMask);
      destination[destinationByteIndex] &= u8(~destinationMask);
      destination[destinationByteIndex] |= u8((destinationBytes << destinationByteOffset) & destinationMask);
      if ((destinationOffset + count - 1) / 8 > destinationByteIndex)
      {
        // The destination spans two bytes
        destination[destinationByteIndex + 1] &= u8(~(destinationMask >> 8));
        destination[destinationByteIndex + 1] |= u8(destinationBytes >> 8);
      }
    }
    else if (destinationByteOffset != sourceByteOffset)
    {
      // Within each byte, the destination needs to be shifted relative to the source. As we visit each source byte, we'll chop off the beginning portion to
      // complete the current destination byte and write it, then chop off the end portion to start the next destination byte.
      s32 leftShift = s32((destinationByteOffset > sourceByteOffset) ? destinationByteOffset - sourceByteOffset : sourceByteOffset - destinationByteOffset);
      s32 rightShift = 8 - leftShift;
      u8 nextDestinationByte = destination[destinationByteIndex];

      {
        u8 destinationMask = u8(~0 << destinationByteOffset);
        nextDestinationByte &= u8(~destinationMask);
        u8 sourceByte = source[sourceByteIndex];
        sourceByteIndex++;

        if (destinationByteOffset > sourceByteOffset)
        {
          // Since the destination byte offset is shifted forward relative to the source byte offset, the first destination byte will need to be masked off
          nextDestinationByte |= u8((sourceByte << leftShift) & destinationMask);
          destination[destinationByteIndex] = nextDestinationByte;
          nextDestinationByte = u8(sourceByte >> rightShift);
          destinationByteIndex++;
        }
        else
        {
          // Since the destination byte offset is shifted backward relative to the source byte offset, the first destination byte will need to be masked off. We
          // won't write this first byte yet because we haven't read enough source bytes, but we'll prepare the masked section.
          nextDestinationByte |= u8(sourceByte >> rightShift) & destinationMask;
        }
      }

      // Read all whole source bytes. Because either the destination byte offset is positive relative to the source byte offset, or the destination byte is
      // lagging behind by 1, we'll always be able to write complete destination bytes.
      usz endWholeSourceByteIndex = (sourceOffset + count) / 8;
      while (sourceByteIndex < endWholeSourceByteIndex)
      {
        u8 sourceByte = source[sourceByteIndex];
        nextDestinationByte |= u8(sourceByte << leftShift);
        destination[destinationByteIndex] = nextDestinationByte;
        nextDestinationByte = u8(sourceByte >> rightShift);
        sourceByteIndex++;
        destinationByteIndex++;
      }

      // When we reach the last source byte, we may or may not yet be on the last destination byte, so we need special logic to handle that
      if (endWholeSourceByteIndex < endSourceByteIndex)
      {
        u8 sourceByte = source[sourceByteIndex];
        nextDestinationByte |= u8(sourceByte << leftShift);

        if (destinationByteIndex + 1 == endDestinationByteIndex)
        {
          // This is also the last destination byte so we need to apply extra masking
          u8 destinationMask = u8(~(~0 << ((destinationOffset + count) % 8)));
          destination[destinationByteIndex] &= u8(~destinationMask);
          destination[destinationByteIndex] |= u8(nextDestinationByte & destinationMask);
        }
        else
        {
          // There is one more destination byte so we can proceed as before
          ASSERT(destinationByteIndex + 2 == endDestinationByteIndex);
          destination[destinationByteIndex] = nextDestinationByte;
          nextDestinationByte = u8(sourceByte >> rightShift);
          destinationByteIndex++;
        }
      }

      // Check if there's one more destination byte we need to write with masking
      if (destinationByteIndex < endDestinationByteIndex)
      {
        ASSERT(destinationByteIndex + 1 == endDestinationByteIndex);
        u8 destinationMask = u8(~(~0 << ((destinationOffset + count) % 8)));
        destination[destinationByteIndex] &= u8(~destinationMask);
        destination[destinationByteIndex] |= u8(nextDestinationByte & destinationMask);
      }
    }
    else
    {
      // No shifting is necessary so we can simply iterate and copy bytes directly, with masking for the first and last ones.
      if (sourceByteOffset > 0)
      {
        u8 mask = u8(~0 << destinationByteOffset);
        u8 sourceByte = source[sourceByteIndex];
        destination[destinationByteIndex] &= u8(~mask);
        destination[destinationByteIndex] |= u8(sourceByte & mask);
        sourceByteIndex++;
        destinationByteIndex++;
      }

      usz endWholeSourceByteIndex = (sourceOffset + count) / 8;
      while (sourceByteIndex < endWholeSourceByteIndex)
      {
        destination[destinationByteIndex] = source[sourceByteIndex];
        sourceByteIndex++;
        destinationByteIndex++;
      }

      if (endWholeSourceByteIndex < endSourceByteIndex)
      {
        u8 mask = u8(~(~0 << ((destinationOffset + count) % 8)));
        u8 sourceByte = source[sourceByteIndex];
        destination[destinationByteIndex] &= u8(~mask);
        destination[destinationByteIndex] |= u8(sourceByte & mask);
      }
    }
  }

  constexpr void SetBits(Span<u8> destination, usz destinationOffset, bool value, usz count)
  {
    ASSERT(destinationOffset + count <= destination.Count() * 8);

    if (count == 0)
      { return; }

    usz destinationByteIndex = destinationOffset / 8;
    usz destinationByteOffset = destinationOffset % 8;
    usz endDestinationByteIndex = (destinationOffset + count + 7) / 8;

    if (destinationByteIndex == endDestinationByteIndex)
    {
      ASSERT(count <= 8);
      u8 mask = u8(~(~0 << count) << destinationByteOffset);
      if (value)
        { destination[destinationByteIndex] |= mask; }
      else
        { destination[destinationByteIndex] &= ~mask; }
    }
    else
    {
      if (destinationByteOffset % 8 != 0)
      {
        u8 mask = u8(~0 << destinationByteOffset);
        if (value)
          { destination[destinationByteIndex] |= mask; }
        else
          { destination[destinationByteIndex] &= ~mask; }
        destinationByteIndex++;
      }

      usz endWholeDestinationByteIndex = (destinationOffset + count) / 8;
      u8 byteValue = value ? 0xff : 0;
      while (destinationByteIndex < endWholeDestinationByteIndex)
      {
        destination[destinationByteIndex] = byteValue;
        destinationByteIndex++;
      }

      if (destinationByteIndex < endDestinationByteIndex)
      {
        u8 mask = u8(~(~0 << ((destinationOffset + count) % 8)));
        if (value)
          { destination[destinationByteIndex] |= mask; }
        else
          { destination[destinationByteIndex] &= ~mask; }
      }
    }
  }
}