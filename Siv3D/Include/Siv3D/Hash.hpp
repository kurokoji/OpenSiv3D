﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (C) 2008-2016 Ryo Suzuki
//	Copyright (C) 2016 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# pragma once
# include <type_traits>
# include <memory>
# include "Fwd.hpp"

namespace s3d
{
	namespace Hash
	{
		inline S3D_CONSTEXPR_CPP14 size_t FNV1a(const void* data_, size_t size)
		{
			const uint8* data = static_cast<const uint8*>(data_);

			# if (SIV3D_PLATFORM_PTR_SIZE == 4)
				const size_t offset_basis	= 2166136261U;
				const size_t FNV_prime		= 16777619U;
			# else
				const size_t offset_basis	= 14695981039346656037ULL;
				const size_t FNV_prime		= 1099511628211ULL;
			# endif

			size_t result = offset_basis;
			
			for (size_t next = 0; next < size; ++next)
			{
				result ^= static_cast<size_t>(data[next]);
				result *= FNV_prime;
			}
			
			return result;
		}

		template <class Type, std::enable_if_t<std::is_trivially_copyable<Type>::value>* = nullptr>
		S3D_CONSTEXPR_CPP14 size_t FNV1a(const Type& keyValue)
		{
			return FNV1a(static_cast<const uint8*>(static_cast<const void*>(std::addressof(keyValue))), sizeof(keyValue));
		}

		namespace detail
		{
			//-----------------------------------------------------------------------------
			// MurmurHash2 was written by Austin Appleby, and is placed in the public
			// domain. The author hereby disclaims copyright to this source code.
			// https://github.com/aappleby/smhasher

			# if defined (SIV3D_TARGET_X86)

				inline S3D_CONSTEXPR_CPP14 uint32 MurmurHash2(const void* key, int32 len, uint32 seed)
				{
					// 'm' and 'r' are mixing constants generated offline.
					// They're not really 'magic', they just happen to work well.

					const uint32 m = 0x5bd1e995;
					const int r = 24;

					// Initialize the hash to a 'random' value

					uint32 h = seed ^ len;

					// Mix 4 bytes at a time into the hash

					const uint8* data = static_cast<const uint8*>(key);

					while (len >= 4)
					{
						uint32 k = *(uint32*)data;

						k *= m;
						k ^= k >> r;
						k *= m;

						h *= m;
						h ^= k;

						data += 4;
						len -= 4;
					}

					// Handle the last few bytes of the input array

					switch (len)
					{
					case 3: h ^= data[2] << 16;
					case 2: h ^= data[1] << 8;
					case 1: h ^= data[0];
						h *= m;
					};

					// Do a few final mixes of the hash to ensure the last few
					// bytes are well-incorporated.

					h ^= h >> 13;
					h *= m;
					h ^= h >> 15;

					return h;
				}

			# else

				inline S3D_CONSTEXPR_CPP14 uint64 MurmurHash64A(const void* key, int32 len, uint64 seed)
				{
					const uint64 m = 0xc6a4a7935bd1e995ULL;
					const int r = 47;

					uint64 h = seed ^ (len * m);

					const uint64* data = static_cast<const uint64*>(key);
					const uint64* end = data + (len / 8);

					while (data != end)
					{
						uint64 k = *data++;

						k *= m;
						k ^= k >> r;
						k *= m;

						h ^= k;
						h *= m;
					}

					const uint8* data2 = static_cast<const uint8*>(static_cast<const void*>(data));

					switch (len & 7)
					{
					case 7: h ^= uint64(data2[6]) << 48;
					case 6: h ^= uint64(data2[5]) << 40;
					case 5: h ^= uint64(data2[4]) << 32;
					case 4: h ^= uint64(data2[3]) << 24;
					case 3: h ^= uint64(data2[2]) << 16;
					case 2: h ^= uint64(data2[1]) << 8;
					case 1: h ^= uint64(data2[0]);
						h *= m;
					};

					h ^= h >> r;
					h *= m;
					h ^= h >> r;

					return h;
				}

			# endif
		}

		inline S3D_CONSTEXPR_CPP14 size_t Murmur2(const void* p, size_t size)
		{
			constexpr uint32 seed = 11111111;

			# if defined (SIV3D_TARGET_X86)

				return detail::MurmurHash2(p, static_cast<int32>(size), seed);

			# else

				return detail::MurmurHash64A(p, static_cast<int32>(size), seed);

			# endif
		}

		template <class Type, std::enable_if_t<std::is_trivially_copyable<Type>::value>* = nullptr>
		S3D_CONSTEXPR_CPP14 size_t Murmur2(const Type& keyValue)
		{
			return Murmur2(static_cast<const uint8*>(static_cast<const void*>(std::addressof(keyValue))), sizeof(keyValue));
		}
	}

	template <class Type>
	struct FNV1aHash
	{
		size_t operator()(const Type& keyVal) const
		{
			return Hash::FNV1a(keyVal);
		}
	};

	template <class Type>
	struct Murmur2Hash
	{
		size_t operator()(const Type& keyVal) const
		{
			return Hash::Murmur2(keyVal);
		}
	};
}
