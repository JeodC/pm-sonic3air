/*
*	Part of the Oxygen Engine / Sonic 3 A.I.R. software distribution.
*	Copyright (C) 2017-2023 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "oxygen/pch.h"
#include "oxygen/drawing/software/Blitter.h"
#include "oxygen/drawing/software/BlitterHelper.h"


void Blitter::blitColor(const OutputWrapper& output, const Color& color, BlendMode blendMode)
{
	BitmapViewMutable<uint32> outputView(output.mBitmapView, output.mViewportRect);
	if (outputView.isEmpty())
		return;

	// TODO: Support other blend modes
	if (blendMode == BlendMode::OPAQUE || color.a >= 1.0f)
	{
		// No blending, just filling
		BlitterHelper::fillRect(outputView, color);
	}
	else if (color.a > 0.0f)
	{
		// Alpha blending
		BlitterHelper::blendRectAlpha(outputView, color);
	}
}

void Blitter::blitSprite(const OutputWrapper& output, const SpriteWrapper& sprite, Vec2i position, const Options& options)
{
	const Recti outputBoundingBox = applyCropping(output.mViewportRect, Recti(-sprite.mPivot, sprite.mBitmapView.getSize()), position, options);
	if (outputBoundingBox.isEmpty())
		return;

	if (nullptr == options.mTransform)
	{
		const Vec2i innerIndent = outputBoundingBox.getPos() - position + sprite.mPivot;

		// As an optimization, differentiate between whether intermediate processing is needed (and we thus need to make an actual copy of the sprite data) or not
		if (needsIntermediateProcessing(options))
		{
			// Copy only the actually used part from the input sprite into an intermediate bitmap
			BitmapViewMutable<uint32> intermediate = makeTempBitmapAsCopy(sprite.mBitmapView, outputBoundingBox.getSize(), innerIndent);

			// Intermediate processing (like tint color / added color)
			processIntermediateBitmap(intermediate, options);

			// Merge into output (incl. blending and depth test)
			BlitterHelper::mergeIntoOutput(output, outputBoundingBox, intermediate, options);
		}
		else
		{
			// We're not going to make changes to the intermediate bitmap, so just use the sprite data directly
			BitmapView<uint32> intermediate(sprite.mBitmapView, Recti(innerIndent, outputBoundingBox.getSize()));

			// Merge into output (incl. blending and depth test)
			BlitterHelper::mergeIntoOutput(output, outputBoundingBox, intermediate, options);
		}
	}
	else
	{
		// TODO: Add optimizations for "simple" transformations, especially flips
		BitmapViewMutable<uint32> intermediate = makeTempBitmapAsTransformedCopy(outputBoundingBox, sprite, position, options);

		// Intermediate processing (like tint color / added color)
		processIntermediateBitmap(intermediate, options);

		// Merge into output (incl. blending and depth test)
		BlitterHelper::mergeIntoOutput(output, outputBoundingBox, intermediate, options);
	}
}

void Blitter::blitIndexed(const OutputWrapper& output, const IndexedSpriteWrapper& sprite, const PaletteWrapper& palette, Vec2i position, const Options& options)
{
	const Recti outputBoundingBox = applyCropping(output.mViewportRect, Recti(-sprite.mPivot, sprite.mBitmapView.getSize()), position, options);
	if (outputBoundingBox.isEmpty())
		return;

	if (nullptr == options.mTransform)
	{
		const Vec2i innerIndent = outputBoundingBox.getPos() - position + sprite.mPivot;

		// Copy only the actually used part from the input sprite into an intermediate bitmap
		BitmapViewMutable<uint32> intermediate = makeTempBitmapAsCopy(sprite.mBitmapView, palette, outputBoundingBox.getSize(), innerIndent);

		// Intermediate processing (like tint color / added color)
		processIntermediateBitmap(intermediate, options);

		// Merge into output (incl. blending and depth test)
		BlitterHelper::mergeIntoOutput(output, outputBoundingBox, intermediate, options);
	}
	else
	{
		// TODO: Add optimizations for "simple" transformations, especially flips
		BitmapViewMutable<uint32> intermediate = makeTempBitmapAsTransformedCopy(outputBoundingBox, sprite, palette, position, options);

		// Intermediate processing (like tint color / added color)
		processIntermediateBitmap(intermediate, options);

		// Merge into output (incl. blending and depth test)
		BlitterHelper::mergeIntoOutput(output, outputBoundingBox, intermediate, options);
	}
}

BitmapViewMutable<uint32> Blitter::makeTempBitmap(Vec2i size)
{
	mTempBitmapData.resize(size.x * size.y);
	return BitmapViewMutable<uint32>(&mTempBitmapData[0], size);
}

BitmapViewMutable<uint32> Blitter::makeTempBitmapAsCopy(const BitmapView<uint32>& input, Vec2i size, Vec2i innerIndent)
{
	BitmapViewMutable<uint32> result = makeTempBitmap(size);
	for (int y = 0; y < size.y; ++y)
	{
		memcpy(result.getLinePointer(y), input.getPixelPointer(innerIndent.x, innerIndent.y + y), size.x * sizeof(uint32));
	}
	return result;
}

BitmapViewMutable<uint32> Blitter::makeTempBitmapAsCopy(const BitmapView<uint8>& input, const PaletteWrapper& palette, Vec2i size, Vec2i innerIndent)
{
	BitmapViewMutable<uint32> result = makeTempBitmap(size);
	for (int y = 0; y < size.y; ++y)
	{
		uint32* dst = result.getLinePointer(y);
		const uint8* src = input.getPixelPointer(innerIndent.x, innerIndent.y + y);
		for (int x = 0; x < size.x; ++x)
		{
			const uint8 index = *src;
			*dst = (index < palette.mNumEntries) ? palette.mPalette[index] : 0;
			++dst;
			++src;
		}
	}
	return result;
}

BitmapViewMutable<uint32> Blitter::makeTempBitmapAsTransformedCopy(Recti outputBoundingBox, const SpriteWrapper& sprite, Vec2i position, const Options& options)
{
	BitmapViewMutable<uint32> result = makeTempBitmap(outputBoundingBox.getSize());
	switch (options.mSamplingMode)
	{
		case SamplingMode::POINT:
		{
			for (int iy = 0; iy < outputBoundingBox.height; ++iy)
			{
				uint32* dst = result.getPixelPointer(0, iy);
				for (int ix = 0; ix < outputBoundingBox.width; ++ix)
				{
					// Transform into sprite-local coordinates
					const float dx = (float)(outputBoundingBox.x + ix - position.x) + 0.5f;
					const float dy = (float)(outputBoundingBox.y + iy - position.y) + 0.5f;
					const int localX = roundToInt(dx * options.mInvTransform[0] + dy * options.mInvTransform[1] - 0.5f) + sprite.mPivot.x;
					const int localY = roundToInt(dx * options.mInvTransform[2] + dy * options.mInvTransform[3] - 0.5f) + sprite.mPivot.y;
					*dst = BlitterHelper::pointSampling(sprite.mBitmapView, localX, localY);
					++dst;
				}
			}
			break;
		}

		case SamplingMode::BILINEAR:
		{
			const Vec2f floatPivot(sprite.mPivot);
			for (int iy = 0; iy < outputBoundingBox.height; ++iy)
			{
				uint32* dst = result.getPixelPointer(0, iy);
				for (int ix = 0; ix < outputBoundingBox.width; ++ix)
				{
					// Transform into sprite-local coordinates
					const float dx = (float)(outputBoundingBox.x + ix - position.x) + 0.5f;
					const float dy = (float)(outputBoundingBox.y + iy - position.y) + 0.5f;
					const float localX = (dx * options.mInvTransform[0] + dy * options.mInvTransform[1] - 0.5f) + floatPivot.x;
					const float localY = (dx * options.mInvTransform[2] + dy * options.mInvTransform[3] - 0.5f) + floatPivot.y;
					*dst = BlitterHelper::bilinearSampling(sprite.mBitmapView, localX, localY);
					++dst;
				}
			}
			break;
		}
	}
	return result;
}

BitmapViewMutable<uint32> Blitter::makeTempBitmapAsTransformedCopy(Recti outputBoundingBox, const IndexedSpriteWrapper& sprite, const PaletteWrapper& palette, Vec2i position, const Options& options)
{
	BitmapViewMutable<uint32> result = makeTempBitmap(outputBoundingBox.getSize());
	switch (options.mSamplingMode)
	{
		case SamplingMode::POINT:
		{
			for (int iy = 0; iy < outputBoundingBox.height; ++iy)
			{
				uint32* dst = result.getPixelPointer(0, iy);
				for (int ix = 0; ix < outputBoundingBox.width; ++ix)
				{
					// Transform into sprite-local coordinates
					const float dx = (float)(outputBoundingBox.x + ix - position.x) + 0.5f;
					const float dy = (float)(outputBoundingBox.y + iy - position.y) + 0.5f;
					const int localX = roundToInt(dx * options.mInvTransform[0] + dy * options.mInvTransform[1] - 0.5f) + sprite.mPivot.x;
					const int localY = roundToInt(dx * options.mInvTransform[2] + dy * options.mInvTransform[3] - 0.5f) + sprite.mPivot.y;
					*dst = BlitterHelper::pointSampling(sprite.mBitmapView, palette, localX, localY);
					++dst;
				}
			}
			break;
		}

		case SamplingMode::BILINEAR:
		{
			const Vec2f floatPivot(sprite.mPivot);
			for (int iy = 0; iy < outputBoundingBox.height; ++iy)
			{
				uint32* dst = result.getPixelPointer(0, iy);
				for (int ix = 0; ix < outputBoundingBox.width; ++ix)
				{
					// Transform into sprite-local coordinates
					const float dx = (float)(outputBoundingBox.x + ix - position.x) + 0.5f;
					const float dy = (float)(outputBoundingBox.y + iy - position.y) + 0.5f;
					const float localX = (dx * options.mInvTransform[0] + dy * options.mInvTransform[1] - 0.5f) + floatPivot.x;
					const float localY = (dx * options.mInvTransform[2] + dy * options.mInvTransform[3] - 0.5f) + floatPivot.y;
					*dst = BlitterHelper::bilinearSampling(sprite.mBitmapView, palette, localX, localY);
					++dst;
				}
			}
			break;
		}
	}
	return result;
}

Recti Blitter::applyCropping(const Recti& viewportRect, const Recti& spriteRect, const Vec2i& position, const Options& options)
{
	if (spriteRect.isEmpty())
		return Recti();

	// First calculate the sprite's bounding box, taking into account the transformation and all inner rectangles
	Recti uncroppedBoundingBox;
	if (nullptr == options.mTransform)
	{
		uncroppedBoundingBox.setPos(position + spriteRect.getPos());
		uncroppedBoundingBox.setSize(spriteRect.getSize());
	}
	else
	{
		Vec2f min(1e10f, 1e10f);
		Vec2f max(-1e10f, -1e10f);
		const Vec2i size = spriteRect.getSize();
		const Vec2i corners[4] = { Vec2i(0, 0), Vec2i(size.x, 0), Vec2i(0, size.y), size };
		for (int i = 0; i < 4; ++i)
		{
			const Vec2f localCorner = Vec2f(corners[i] + spriteRect.getPos());
			const float screenCornerX = position.x + localCorner.x * options.mTransform[0] + localCorner.y * options.mTransform[1];
			const float screenCornerY = position.y + localCorner.x * options.mTransform[2] + localCorner.y * options.mTransform[3];
			min.x = std::min(screenCornerX, min.x);
			min.y = std::min(screenCornerY, min.y);
			max.x = std::max(screenCornerX, max.x);
			max.y = std::max(screenCornerY, max.y);
		}

		uncroppedBoundingBox.x = (int)min.x;
		uncroppedBoundingBox.y = (int)min.y;
		uncroppedBoundingBox.width  = (int)max.x + 1 - uncroppedBoundingBox.x;
		uncroppedBoundingBox.height = (int)max.y + 1 - uncroppedBoundingBox.y;
	}

	// Get the (cropped) bounding box in the output viewport
	return Recti::getIntersection(uncroppedBoundingBox, viewportRect);
}

bool Blitter::needsIntermediateProcessing(const Options& options)
{
	return (nullptr != options.mTintColor || nullptr != options.mAddedColor || options.mSwapRedBlueChannels);
}

void Blitter::processIntermediateBitmap(BitmapViewMutable<uint32>& bitmap, const Options& options)
{
	if (nullptr != options.mTintColor)
	{
		// Apply tint color
		const uint32 tintColor = options.mTintColor->getABGR32();
		const uint32 mult[4] =
		{
			(uint32)roundToInt(options.mTintColor->r * 0x100),
			(uint32)roundToInt(options.mTintColor->g * 0x100),
			(uint32)roundToInt(options.mTintColor->b * 0x100),
			(uint32)roundToInt(options.mTintColor->a * 0x100)
		};
		for (int y = 0; y < bitmap.getSize().y; ++y)
		{
			uint8* dst = (uint8*)bitmap.getLinePointer(y);
			for (int x = 0; x < bitmap.getSize().x; ++x)
			{
				dst[0] = (uint8)std::min<uint32>((dst[0] * mult[0]) >> 8, 0xff);
				dst[1] = (uint8)std::min<uint32>((dst[1] * mult[1]) >> 8, 0xff);
				dst[2] = (uint8)std::min<uint32>((dst[2] * mult[2]) >> 8, 0xff);
				dst[3] = (uint8)std::min<uint32>((dst[3] * mult[3]) >> 8, 0xff);
				dst += 4;
			}
		}
	}

	if (nullptr != options.mAddedColor)
	{
		// Apply added color
		const uint8 add[3] =
		{
			(uint8)roundToInt(options.mAddedColor->r * 0xff),
			(uint8)roundToInt(options.mAddedColor->g * 0xff),
			(uint8)roundToInt(options.mAddedColor->b * 0xff)
		};
		for (int y = 0; y < bitmap.getSize().y; ++y)
		{
			uint8* dst = (uint8*)bitmap.getLinePointer(y);
			for (int x = 0; x < bitmap.getSize().x; ++x)
			{
				dst[0] = std::min(dst[0] + add[0], 0xff);
				dst[1] = std::min(dst[1] + add[1], 0xff);
				dst[2] = std::min(dst[2] + add[2], 0xff);
				dst += 4;
			}
		}
	}

	if (options.mSwapRedBlueChannels)
	{
		// Copy over data and swap red and blue channels
		const int numPixels = bitmap.getSize().x;
		for (int y = 0; y < bitmap.getSize().y; ++y)
		{
			uint32* dst = bitmap.getLinePointer(y);
			int k = 0;
			if constexpr (sizeof(void*) == 8)
			{
				// On 64-bit architectures: Process 2 pixels at once
				for (; k < numPixels; k += 2)
				{
					const uint64 colors = *(uint64*)dst;
					*(uint64*)dst = ((colors & 0x00ff000000ff0000ull) >> 16) | (colors & 0xff00ff00ff00ff00ull) | ((colors & 0x000000ff000000ffull) << 16);
					dst += 2;
				}
			}
			// Process single pixels
			for (; k < numPixels; ++k)
			{
				const uint32 color = *dst;
				*dst = ((color & 0x00ff0000) >> 16) | (color & 0xff00ff00) | ((color & 0x000000ff) << 16);
				++dst;
			}
		}
	}
}
