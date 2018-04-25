/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
/****************************************************************************
// A buffer that loops round and overwrites old data (e.g. for the debug strings)
// FIXME: Could really use some cleanup, move over to usagi conventions
*****************************************************************************/
#ifndef __USG_DEBUG_CIRCULAR_BUFFER_H__
#define __USG_DEBUG_CIRCULAR_BUFFER_H__

namespace usg
{

	template<int S> 
	class CircularBuffer
	{
	public:
		CircularBuffer():m_writePos(0),m_readPos(0){}

		void Write(const char *s)
		{
			while (*s)
			{
				m_buffer[m_writePos % S] = *s++;
				m_writePos++;
			}
		}

		void Clear()
		{
			m_writePos = 0;
			m_readPos = 0;
			memset(m_buffer,0,S);
		}

		int	GetRemainder() 
		{
			return m_writePos - m_readPos;
		}

		int	GetRemainderBlock() 
		{
			int i = m_readPos % S;
			int left = GetRemainder();
			if ((i+left) > S)
				left = S - i;
			return left;
		}

		const char* Read(int len) 
		{
			const char *p = &m_buffer[m_readPos % S];
			m_readPos +=len;
			return p;
		}

		const char* GetReadBuffer()
		{
			return &m_buffer[m_readPos % S];
		}

		void RewindReadPos()
		{
			m_readPos = m_writePos - S;
			if (m_readPos < 0)
				m_readPos = 0;
		}

		void SetReadToLastLines(int num)
		{
			m_readPos = m_writePos;
			while (num && m_readPos>0 && ((m_writePos-m_readPos) < S))
			{
				if (m_buffer[m_readPos%S] == '\n')
					num--;

				m_readPos--;
			}
		}

		void SetWritePos(int p)
		{
			m_writePos = p;
		}

		char ReadChar()
		{
			return *Read(1);
		}


		int FillStringBuffer(char *buf,int maxlen, int width)
		{
			int i=0;
			int x = 0;
			while (GetRemainder() && i<maxlen)
			{
				char c;

				if (x>=width)
					c = '\n';
				else
					c = ReadChar();

				if (c == 0)
					c = ' ';

				if (c == '\n')
					x = 0;
				else
					x++;

				buf[i++] = c;

			}
			buf[i] = 0;
			return i;
		}

	private:
		char	m_buffer[S];
		int		m_writePos;
		int		m_readPos;
	};

};

#endif