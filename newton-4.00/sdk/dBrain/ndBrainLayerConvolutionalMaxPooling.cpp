/* Copyright (c) <2003-2022> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "ndBrainStdafx.h"
#include "ndBrainSaveLoad.h"
#include "ndBrainLayerConvolutionalMaxPooling.h"

ndBrainLayerConvolutionalMaxPooling::ndBrainLayerConvolutionalMaxPooling(ndInt32 inputWidth, ndInt32 inputHeight, ndInt32 inputDepth)
	:ndBrainLayerActivation(((inputWidth + 1) / 2) * ((inputHeight + 1) / 2) * inputDepth)
	,m_index()
	,m_width(inputWidth)
	,m_height(inputHeight)
	,m_channels(inputDepth)
{
	m_index.SetCount(m_neurons);
}

ndBrainLayerConvolutionalMaxPooling::ndBrainLayerConvolutionalMaxPooling(const ndBrainLayerConvolutionalMaxPooling& src)
	:ndBrainLayerActivation(src)
	,m_index()
	,m_width(src.m_width)
	,m_height(src.m_height)
	,m_channels(src.m_channels)
{
	m_index.SetCount(src.m_index.GetCount());
}

ndInt32 ndBrainLayerConvolutionalMaxPooling::GetInputWidth() const
{
	return m_width;
}

ndInt32 ndBrainLayerConvolutionalMaxPooling::GetInputHeight() const
{
	return m_height;
}

ndInt32 ndBrainLayerConvolutionalMaxPooling::GetInputChannels() const
{
	return m_channels;
}

ndInt32 ndBrainLayerConvolutionalMaxPooling::GetOutputWidth() const
{
	return (m_width + 1) / 2;
}

ndInt32 ndBrainLayerConvolutionalMaxPooling::GetOutputHeight() const
{
	return (m_height + 1) / 2;
}

ndInt32 ndBrainLayerConvolutionalMaxPooling::GetOutputChannels() const
{
	return m_channels;
}

ndInt32 ndBrainLayerConvolutionalMaxPooling::GetInputSize() const
{
	return m_width * m_height * m_channels;
}

ndBrainLayer* ndBrainLayerConvolutionalMaxPooling::Clone() const
{
	return new ndBrainLayerConvolutionalMaxPooling(*this);
}

const char* ndBrainLayerConvolutionalMaxPooling::GetLabelId() const
{
	return "ndBrainLayerConvolutionalMaxPooling";
}

ndBrainLayer* ndBrainLayerConvolutionalMaxPooling::Load(const ndBrainLoad* const loadSave)
{
	ndAssert(0);
	return nullptr;
	//char buffer[1024];
	//loadSave->ReadString(buffer);
	//
	//loadSave->ReadString(buffer);
	//ndInt32 inputs = loadSave->ReadInt();
	//ndBrainLayerConvolutionalMaxPooling* const layer = new ndBrainLayerConvolutionalMaxPooling(inputs);
	//loadSave->ReadString(buffer);
	//return layer;
}

void ndBrainLayerConvolutionalMaxPooling::InputDerivative(const ndBrainVector& output, const ndBrainVector& outputDerivative, ndBrainVector& inputDerivative) const
{
	ndAssert(output.GetCount() == outputDerivative.GetCount());
	ndAssert(m_index.GetCount() == outputDerivative.GetCount());

	inputDerivative.Set(ndBrainFloat(0.0f));
	for (ndInt32 i = m_index.GetCount() - 1; i >= 0; --i)
	{
		ndInt32 index = m_index[i];
		inputDerivative[index] = outputDerivative[i];
	}
}


void ndBrainLayerConvolutionalMaxPooling::MakePrediction(const ndBrainVector& input, ndBrainVector& output) const
{
	ndAssert(input.GetCount() == GetInputSize());
	ndAssert(output.GetCount() == GetOutputSize());

	//ndInt32 baseOut___ = 0;
	//for (ndInt32 k = 0; k < m_channels; ++k)
	//{
	//	const ndInt32 base = k * m_height * m_width;
	//	const ndBrainMemVector in(&input[base], m_height * m_width);
	//
	//	ndInt32 baseIn = 0;
	//	for (ndInt32 i = 0; i < (m_height & -2); i += 2)
	//	{
	//		for (ndInt32 j = 0; j < (m_width & -2); j += 2)
	//		{
	//			ndInt32 index = baseIn + j;
	//			ndBrainFloat maxValue = in[index];
	//			if (in[baseIn + j + 1] > maxValue)
	//			{
	//				index = baseIn + j + 1;
	//				maxValue = in[index];
	//			}
	//			if (in[baseIn + m_width + j] > maxValue)
	//			{
	//				index = baseIn + m_width + j;
	//				maxValue = in[index];
	//			}
	//			if (in[baseIn + m_width + j + 1] > maxValue)
	//			{
	//				index = baseIn + m_width + j + 1;
	//				maxValue = in[index];
	//			}
	//			output[baseOut___ + (j >> 1)] = maxValue;
	//			m_index[baseOut___ + (j >> 1)] = base + index;
	//		}
	//
	//		if (m_width & 1)
	//		{
	//			ndInt32 index = baseIn + m_width - 1;
	//			ndBrainFloat maxValue = in[index];
	//			if (in[baseIn + m_width + m_width - 1] > maxValue)
	//			{
	//				index = baseIn + m_width + m_width - 1;
	//				maxValue = in[index];
	//			}
	//			output[baseOut___ + (m_width >> 1)] = maxValue;
	//			m_index[baseOut___ + (m_width >> 1)] = base + index;
	//		}
	//
	//		baseIn += m_width * 2;
	//		baseOut___ += (m_width + 1) >> 1;
	//	}
	//
	//	if (m_height & 1)
	//	{
	//		for (ndInt32 j = 0; j < (m_width & -2); j += 2)
	//		{
	//			ndInt32 index = baseIn + j;
	//			ndBrainFloat maxValue = in[index];
	//			if (in[baseIn + j + 1] > maxValue)
	//			{
	//				index = baseIn + j + 1;
	//				maxValue = in[index];
	//			}
	//			output[baseOut___ + (j >> 1)] = maxValue;
	//			m_index[baseOut___ + (j >> 1)] = base + index;
	//		}
	//
	//		if (m_width & 1)
	//		{
	//			ndInt32 index = baseIn + m_width - 1;
	//			ndBrainFloat maxValue = in[index];
	//			output[baseOut___ + (m_width >> 1)] = maxValue;
	//			m_index[baseOut___ + (m_width >> 1)] = base + index;
	//		}
	//
	//		baseIn += m_width * 2;
	//		baseOut___ += (m_width + 1) >> 1;
	//	}
	//}

	
	const ndBrainFloat minValue = ndBrainFloat(-1.0e20f);
	const ndInt32 inputSize = m_height * m_width;

	ndInt32 offsetOut = 0;
	ndInt32 inputOffset = 0;
	for (ndInt32 k = 0; k < m_channels; ++k)
	{
		ndInt32 inputStride = 0;
		const ndBrainMemVector in(&input[inputOffset], inputSize);
		for (ndInt32 y = 0; y < m_height; y += 2)
		{
			ndInt32 yMask = (y + 1) < m_width;
			for (ndInt32 x = 0; x < m_width; x += 2)
			{
				const ndInt32 xMask = (x + 1) < m_width;

				const ndInt32 x0 = inputStride + x;
				const ndInt32 x1 = x0 + 1;
				const ndInt32 x2 = x0 + m_width;
				const ndInt32 x3 = x2 + 1;

				//ndBrainFloat block[4];
				//block[0] = in[x0];
				//block[1] = xMask ? in[x1] : minValue;
				//block[2] = yMask ? in[x2] : minValue;
				//block[3] = (xMask & yMask) ? in[x3] : minValue;
				//ndInt32 index = x0;
				//ndBrainFloat maxValue = block[0];
				//if (block[1] > maxValue)
				//{
				//	index = x1;
				//	maxValue = block[1];
				//}
				//if (block[2] > maxValue)
				//{
				//	index = x2;
				//	maxValue = block[2];
				//}
				//if (block[3] > maxValue)
				//{
				//	index = x3;
				//	maxValue = block[3];
				//}
				//output[offsetOut + (x >> 1)] = maxValue;
				//m_index[offsetOut + (x >> 1)] = inputOffset + index;

				const ndBrainFloat val0 = in[x0];
				const ndBrainFloat val1 = xMask ? in[x1] : minValue;
				const ndBrainFloat val2 = yMask ? in[x2] : minValue;
				const ndBrainFloat val3 = (xMask & yMask) ? in[x3] : minValue;

				const bool test01 = val0 >= val1;
				const ndInt32 index01 = test01 ? x0 : x1;
				const ndBrainFloat val01 = test01 ? val0 : val1;

				const bool test23 = val2 >= val3;
				const ndInt32 index23 = test23 ? x2 : x3;
				const ndBrainFloat val23 = test23 ? val2 : val3;

				const bool test0123 = val01 >= val23;
				const ndInt32 index0123 = test0123 ? index01 : index23;
				const ndBrainFloat val0123 = test0123 ? val01 : val23;
				
				//ndAssert(index0123 == index);

				output[offsetOut + (x >> 1)] = val0123;
				m_index[offsetOut + (x >> 1)] = inputOffset + index0123;
			}

			inputStride += m_width * 2;
			offsetOut += (m_width + 1) >> 1;
		}

		inputOffset += inputSize;
	}
}