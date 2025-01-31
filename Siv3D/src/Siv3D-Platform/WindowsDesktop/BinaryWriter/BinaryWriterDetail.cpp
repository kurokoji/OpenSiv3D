﻿//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2019 Ryo Suzuki
//	Copyright (c) 2016-2019 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include <Siv3D/EngineLog.hpp>
# include "BinaryWriterDetail.hpp"

namespace s3d
{
	BinaryWriter::BinaryWriterDetail::BinaryWriterDetail()
	{

	}

	BinaryWriter::BinaryWriterDetail::~BinaryWriterDetail()
	{
		close();
	}

	int64 BinaryWriter::BinaryWriterDetail::fillBuffer(const void* src, const size_t size)
	{
		if (!m_pBuffer)
		{
			m_pBuffer = static_cast<Byte*>(std::malloc(BufferSize));
		}

		std::memcpy(m_pBuffer + m_currentBufferPos, src, size);

		m_currentBufferPos += size;

		return size;
	}

	bool BinaryWriter::BinaryWriterDetail::open(const FilePathView path, const OpenMode openMode)
	{
		if (m_opened)
		{
			close();
		}

		if (path.isEmpty())
		{
			LOG_FAIL(U"❌ BinaryWriter: path is empty");

			return false;
		}

		FilePath fullPath;

		const FilePath parentFilePath = FileSystem::ParentPath(path, 0, &fullPath);

		if (!FileSystem::Exists(parentFilePath) && !FileSystem::CreateDirectories(parentFilePath))
		{
			LOG_FAIL(U"❌ BinaryWriter: Failed to create parent directories \"{0}\""_fmt(parentFilePath));

			return false;
		}

		m_handle = ::CreateFileW(Unicode::ToWString(path).c_str(), GENERIC_WRITE, 0, nullptr,
			((openMode == OpenMode::Append) ? OPEN_ALWAYS : CREATE_ALWAYS),
			FILE_ATTRIBUTE_NORMAL, nullptr);

		m_opened = (m_handle != INVALID_HANDLE_VALUE);

		if (!m_opened)
		{
			LOG_FAIL(U"❌ BinaryWriter: Failed to open file \"{0}\""_fmt(path));

			return false;
		}

		if (openMode == OpenMode::Append)
		{
			LARGE_INTEGER distance = { 0, 0 };

			::SetFilePointerEx(m_handle, distance, nullptr, FILE_END);
		}

		m_fullPath = fullPath;

		LOG_DEBUG(U"📤 BinaryReader: Opened file \"{0}\""_fmt(m_fullPath));

		return true;
	}

	void BinaryWriter::BinaryWriterDetail::flush()
	{
		if (m_currentBufferPos == 0)
		{
			return;
		}

		DWORD written = 0;

		::WriteFile(m_handle, m_pBuffer, static_cast<uint32>(m_currentBufferPos), &written, nullptr);

		m_currentBufferPos = 0;
	}

	void BinaryWriter::BinaryWriterDetail::close()
	{
		if (!m_opened)
		{
			return;
		}

		flush();

		std::free(m_pBuffer);

		m_pBuffer = nullptr;

		::CloseHandle(m_handle);

		m_handle = INVALID_HANDLE_VALUE;

		m_opened = false;

		LOG_DEBUG(U"📥 BinaryWriter: Closed file \"{0}\""_fmt(m_fullPath));

		m_fullPath.clear();
	}

	bool BinaryWriter::BinaryWriterDetail::isOpen() const noexcept
	{
		return m_opened;
	}

	void BinaryWriter::BinaryWriterDetail::clear()
	{
		if (!m_opened)
		{
			return;
		}
        
        m_currentBufferPos = 0;

		setPos(0);

		::SetEndOfFile(m_handle);
	}

	int64 BinaryWriter::BinaryWriterDetail::size()
	{
        if (!m_opened)
        {
            return 0;
        }
        
		flush();

		return FileSystem::Size(m_fullPath);
	}

	int64 BinaryWriter::BinaryWriterDetail::setPos(int64 pos)
	{
        if (!m_opened)
        {
            return 0;
        }
        
		flush();

		LARGE_INTEGER distance, newPos;

		distance.QuadPart = pos;

		::SetFilePointerEx(m_handle, distance, &newPos, FILE_BEGIN);

		return newPos.QuadPart;
	}

	int64 BinaryWriter::BinaryWriterDetail::getPos()
	{
        if (!m_opened)
        {
            return 0;
        }
        
		flush();

		LARGE_INTEGER distance = { 0, 0 }, currentPos;

		::SetFilePointerEx(m_handle, distance, &currentPos, FILE_CURRENT);

		return currentPos.QuadPart;
	}

	int64 BinaryWriter::BinaryWriterDetail::write(const void* src, size_t size)
	{
        if (!m_opened || size == 0)
        {
            return 0;
        }
        
		assert(src != nullptr);

		if (size < BufferSize)
		{
			if (m_currentBufferPos + size > BufferSize)
			{
				flush();
			}

			return fillBuffer(src, size);
		}
		else
		{
			flush();
		}

		DWORD writtenBytes;

		::WriteFile(m_handle, src, static_cast<uint32>(size), &writtenBytes, nullptr);

		return writtenBytes;
	}

	const FilePath& BinaryWriter::BinaryWriterDetail::path() const
	{
		return m_fullPath;
	}
}
