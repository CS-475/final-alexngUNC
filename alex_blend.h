#ifndef alex_blend_DEFINED
#define alex_blend_DEFINED

static inline void clearRow(GPixel row[], int count) {
	for (int i=0; i<count; ++i) {
		row[i] = 0;
	}
}

static inline void clearRowSrc(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = 0;
	}
}

static inline void storeRow(GPixel row[], int count, GPixel color) {
    for (int i = 0; i < count; ++i) {
        row[i] = color;
    }
}

// SR = SrcRow
static inline void storeRowSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = srcRow[i];
	}
}

static inline GPixel srcOver(GPixel dst, GPixel src) {
	// read color from destination
	int d_alpha = GPixel_GetA(dst);
	int d_red = GPixel_GetR(dst);
	int d_green = GPixel_GetG(dst);
	int d_blue = GPixel_GetB(dst);

	// read color from source
	int s_alpha = GPixel_GetA(src);
	int s_red = GPixel_GetR(src);
	int s_green = GPixel_GetG(src);
	int s_blue = GPixel_GetB(src);

	// blend color with dest
	unsigned r_alpha = 0;
	unsigned r_red = 0;
	unsigned r_green = 0;
	unsigned r_blue = 0;

	r_alpha = s_alpha + div_255((255 - s_alpha) * d_alpha);
	r_red = s_red + div_255((255 - s_alpha) * d_red);
	r_green = s_green + div_255((255 - s_alpha) * d_green);
	r_blue = s_blue + div_255((255 - s_alpha) * d_blue);
	return GPixel_PackARGB(r_alpha, r_red, r_green, r_blue);
}

static inline GPixel dstOver(GPixel dst, GPixel src) {
	// read color from destination
	int d_alpha = GPixel_GetA(dst);
	int d_red = GPixel_GetR(dst);
	int d_green = GPixel_GetG(dst);
	int d_blue = GPixel_GetB(dst);

	// read color from source
	int s_alpha = GPixel_GetA(src);
	int s_red = GPixel_GetR(src);
	int s_green = GPixel_GetG(src);
	int s_blue = GPixel_GetB(src);

	// blend color with dest
	unsigned r_alpha = 0;
	unsigned r_red = 0;
	unsigned r_green = 0;
	unsigned r_blue = 0;

	r_alpha = d_alpha + div_255((255 - d_alpha) * s_alpha);
	r_red = d_red + div_255((255 - d_alpha) * s_red);
	r_green = d_green + div_255((255 - d_alpha) * s_green);
	r_blue = d_blue + div_255((255 - d_alpha) * s_blue);
	return GPixel_PackARGB(r_alpha, r_red, r_green, r_blue);
}

static inline GPixel srcIn(GPixel dst, GPixel src) {
	// read color from destination
	int d_alpha = GPixel_GetA(dst);

	// read color from source
	int s_alpha = GPixel_GetA(src);
	int s_red = GPixel_GetR(src);
	int s_green = GPixel_GetG(src);
	int s_blue = GPixel_GetB(src);

	// blend color with dest
	unsigned r_alpha = 0;
	unsigned r_red = 0;
	unsigned r_green = 0;
	unsigned r_blue = 0;

	r_alpha = div_255(d_alpha * s_alpha); 
	r_red = div_255(d_alpha * s_red);
	r_green = div_255(d_alpha * s_green);
	r_blue = div_255(d_alpha * s_blue);
	return GPixel_PackARGB(r_alpha, r_red, r_green, r_blue);
}

static inline GPixel dstIn(GPixel dst, GPixel src) {
	// read color from destination
	int d_alpha = GPixel_GetA(dst);
	int d_red = GPixel_GetR(dst);
	int d_green = GPixel_GetG(dst);
	int d_blue = GPixel_GetB(dst);

	// read color from source
	int s_alpha = GPixel_GetA(src);

	// blend color with dest
	unsigned r_alpha = 0;
	unsigned r_red = 0;
	unsigned r_green = 0;
	unsigned r_blue = 0;

	r_alpha = div_255(s_alpha * d_alpha); 
	r_red = div_255(s_alpha * d_red);
	r_green = div_255(s_alpha * d_green);
	r_blue = div_255(s_alpha * d_blue);
	return GPixel_PackARGB(r_alpha, r_red, r_green, r_blue);
}

static inline GPixel srcOut(GPixel dst, GPixel src) {
	// read color from destination
	int d_alpha = GPixel_GetA(dst);

	// read color from source
	int s_alpha = GPixel_GetA(src);
	int s_red = GPixel_GetR(src);
	int s_green = GPixel_GetG(src);
	int s_blue = GPixel_GetB(src);

	// blend color with dest
	unsigned r_alpha = 0;
	unsigned r_red = 0;
	unsigned r_green = 0;
	unsigned r_blue = 0;

	r_alpha = div_255((255 - d_alpha) * s_alpha);
	r_red = div_255((255 - d_alpha) * s_red);	
	r_green = div_255((255 - d_alpha) * s_green);
	r_blue = div_255((255 - d_alpha) * s_blue);

	return GPixel_PackARGB(r_alpha, r_red, r_green, r_blue);
}

static inline GPixel dstOut(GPixel dst, GPixel src) {
	// read color from destination
	int d_alpha = GPixel_GetA(dst);
	int d_red = GPixel_GetR(dst);
	int d_green = GPixel_GetG(dst);
	int d_blue = GPixel_GetB(dst);

	// read color from source
	int s_alpha = GPixel_GetA(src);

	// blend color with dest
	unsigned r_alpha = 0;
	unsigned r_red = 0;
	unsigned r_green = 0;
	unsigned r_blue = 0;

	r_alpha = div_255((255 - s_alpha) * d_alpha);
	r_red = div_255((255 - s_alpha) * d_red);	
	r_green = div_255((255 - s_alpha) * d_green);
	r_blue = div_255((255 - s_alpha) * d_blue);	

	return GPixel_PackARGB(r_alpha, r_red, r_green, r_blue);
}

static inline GPixel srcATop(GPixel dst, GPixel src) {
	// read color from destination
	int d_alpha = GPixel_GetA(dst);
	int d_red = GPixel_GetR(dst);
	int d_green = GPixel_GetG(dst);
	int d_blue = GPixel_GetB(dst);

	// read color from source
	int s_alpha = GPixel_GetA(src);
	int s_red = GPixel_GetR(src);
	int s_green = GPixel_GetG(src);
	int s_blue = GPixel_GetB(src);

	// blend color with dest
	unsigned r_alpha = 0;
	unsigned r_red = 0;
	unsigned r_green = 0;
	unsigned r_blue = 0;

	r_alpha = div_255(d_alpha * s_alpha + (255 - s_alpha) * d_alpha);
	r_red = div_255(d_alpha * s_red + (255 - s_alpha) * d_red);	
	r_green = div_255(d_alpha * s_green + (255 - s_alpha) * d_green);	
	r_blue = div_255(d_alpha * s_blue + (255 - s_alpha) * d_blue);	
	return GPixel_PackARGB(r_alpha, r_red, r_green, r_blue);
}

static inline GPixel dstATop(GPixel dst, GPixel src) {
	// read color from destination
	int d_alpha = GPixel_GetA(dst);
	int d_red = GPixel_GetR(dst);
	int d_green = GPixel_GetG(dst);
	int d_blue = GPixel_GetB(dst);

	// read color from source
	int s_alpha = GPixel_GetA(src);
	int s_red = GPixel_GetR(src);
	int s_green = GPixel_GetG(src);
	int s_blue = GPixel_GetB(src);

	// blend color with dest
	unsigned r_alpha = 0;
	unsigned r_red = 0;
	unsigned r_green = 0;
	unsigned r_blue = 0;

	r_alpha = div_255(s_alpha * d_alpha + (255 - d_alpha) * s_alpha);
	r_red = div_255(s_alpha * d_red + (255 - d_alpha) * s_red);	
	r_green = div_255(s_alpha * d_green + (255 - d_alpha) * s_green);	
	r_blue = div_255(s_alpha * d_blue + (255 - d_alpha) * s_blue);		

	return GPixel_PackARGB(r_alpha, r_red, r_green, r_blue);
}

static inline GPixel xorBlend(GPixel dst, GPixel src) {
	// read color from destination
	int d_alpha = GPixel_GetA(dst);
	int d_red = GPixel_GetR(dst);
	int d_green = GPixel_GetG(dst);
	int d_blue = GPixel_GetB(dst);

	// read color from source
	int s_alpha = GPixel_GetA(src);
	int s_red = GPixel_GetR(src);
	int s_green = GPixel_GetG(src);
	int s_blue = GPixel_GetB(src);

	// blend color with dest
	unsigned r_alpha = 0;
	unsigned r_red = 0;
	unsigned r_green = 0;
	unsigned r_blue = 0;

	r_alpha = div_255((255 - s_alpha) * d_alpha + (255 - d_alpha) * s_alpha);
	r_red = div_255((255 - s_alpha) * d_red + (255 - d_alpha) * s_red);
	r_green = div_255((255 - s_alpha) * d_green + (255 - d_alpha) * s_green);
	r_blue = div_255((255 - s_alpha) * d_blue + (255 - d_alpha) * s_blue);

	return GPixel_PackARGB(r_alpha, r_red, r_green, r_blue);
}

static inline GBlendMode optimizeBlendMode(GBlendMode bm, GPixel src) {
	unsigned alpha = GPixel_GetA(src);
	switch (bm) {
		case GBlendMode::kClear:
			return bm;
		case GBlendMode::kSrc:
			if (alpha == 0)
				return GBlendMode::kClear;
			return bm;
		case GBlendMode::kDst:
			if (alpha == 0)
				return GBlendMode::kClear;
			return bm;
		case GBlendMode::kSrcOver:
			if (alpha == 255)
				return GBlendMode::kSrc;
			else if (alpha == 0)
				return GBlendMode::kDst;
			return bm;
		case GBlendMode::kDstOver:
			if (alpha == 0)
				return GBlendMode::kDst;
			return bm;
		case GBlendMode::kSrcIn:
			if (alpha == 0)
				return GBlendMode::kClear;
			return bm;
		case GBlendMode::kDstIn:
			if (alpha == 255)
				return GBlendMode::kDst;
			else if (alpha == 0)
				return GBlendMode::kClear;
			return bm;
		case GBlendMode::kSrcOut:
			if (alpha == 0)
				return GBlendMode::kClear;
			return bm;
		case GBlendMode::kDstOut:
			if (alpha == 255)
				return GBlendMode::kClear;
			else if (alpha == 0)
				return GBlendMode::kDst;
			return bm;
		case GBlendMode::kSrcATop:
			if (alpha == 255)
				return GBlendMode::kSrcIn;
			else if (alpha == 0)
				return GBlendMode::kDst;
			return bm;
		case GBlendMode::kDstATop:
			if (alpha == 1)
				return GBlendMode::kDstOver;
			else if (alpha == 0)
				return GBlendMode::kClear;
			return bm;
		case GBlendMode::kXor:
			if (alpha == 255)
				return GBlendMode::kSrcOut;
			else if (alpha == 0)
				return GBlendMode::kDst;
			return bm;
		default:
			return bm;
	}
}

static inline GBlendMode optimizeOpaqueBlendMode(GBlendMode bm) {
	switch (bm) {
		case GBlendMode::kClear:
			return bm;
		case GBlendMode::kSrc:
			return bm;
		case GBlendMode::kDst:
			return bm;
		case GBlendMode::kSrcOver:
			return GBlendMode::kSrc;
		case GBlendMode::kDstOver:
			return bm;
		case GBlendMode::kSrcIn:
			return bm;
		case GBlendMode::kDstIn:
			return GBlendMode::kDst;
		case GBlendMode::kSrcOut:
			return bm;
		case GBlendMode::kDstOut:
			return GBlendMode::kClear;
		case GBlendMode::kSrcATop:
			return GBlendMode::kSrcIn;
		case GBlendMode::kDstATop:
			return GBlendMode::kDstOver;
		case GBlendMode::kXor:
			return GBlendMode::kSrcOut;
		default:
			return bm;
	}
}

static inline void blitSrcOver(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = srcOver(row[i], src);
	}
}

static inline void blitSrcOverSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = srcOver(row[i], srcRow[i]);
	}
}

static inline void blitDstOver(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = dstOver(row[i], src);
	}
}

static inline void blitDstOverSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = dstOver(row[i], srcRow[i]);
	}
}

static inline void blitSrcIn(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = srcIn(row[i], src);
	}
}

static inline void blitSrcInSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = srcIn(row[i], srcRow[i]);
	}
}

static inline void blitDstIn(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = dstIn(row[i], src);
	}
}

static inline void blitDstInSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = dstIn(row[i], srcRow[i]);
	}
}

static inline void blitSrcOut(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = srcOut(row[i], src);
	}
}

static inline void blitSrcOutSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = srcOut(row[i], srcRow[i]);
	}
}

static inline void blitDstOut(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = dstOut(row[i], src);
	}
}

static inline void blitDstOutSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = dstOut(row[i], srcRow[i]);
	}
}

static inline void blitSrcATop(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = srcATop(row[i], src);
	}
}

static inline void blitSrcATopSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = srcATop(row[i], srcRow[i]);
	}
}

static inline void blitDstATop(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = dstATop(row[i], src);
	}
}

static inline void blitDstATopSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = dstATop(row[i], srcRow[i]);
	}
}

static inline void blitXorBlend(GPixel row[], int count, GPixel src) {
	for (int i=0; i<count; ++i) {
		row[i] = xorBlend(row[i], src);
	}
}

static inline void blitXorBlendSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = xorBlend(row[i], srcRow[i]);
	}
}

static inline void clearRowSR(GPixel row[], int count, GPixel srcRow[]) {
	for (int i=0; i<count; ++i) {
		row[i] = 0;
	}
}

typedef void (*BlitRowProc)(GPixel row[], int count, GPixel src);

const BlitRowProc gblitRowProcs[] = {
    // since our enum values range from 0 … 11, we can prepopulate
    // an array with their corresponding function values.
    clearRowSrc, storeRow, nullptr, blitSrcOver, blitDstOver, 
	blitSrcIn, blitDstIn, blitSrcOut, blitDstOut,
	blitSrcATop, blitDstATop, blitXorBlend
};

typedef void (*BlitRowSRProc)(GPixel row[], int count, GPixel srcRow[]);

const BlitRowSRProc gblitRowSRProcs[] = {
    // since our enum values range from 0 … 11, we can prepopulate
    // an array with their corresponding function values.
    clearRowSR, storeRowSR, nullptr, blitSrcOverSR, blitDstOverSR, 
	blitSrcInSR, blitDstInSR, blitSrcOutSR, blitDstOutSR,
	blitSrcATopSR, blitDstATopSR, blitXorBlendSR
};

static inline GPixel modulateBlend(GPixel p1, GPixel p2) {
	// read color from first pixel
	int a1 = GPixel_GetA(p1);
	int r1 = GPixel_GetR(p1);
	int g1 = GPixel_GetG(p1);
	int b1 = GPixel_GetB(p1);

	// read color from second pixel
	int a2 = GPixel_GetA(p2);
	int r2 = GPixel_GetR(p2);
	int g2 = GPixel_GetG(p2);
	int b2 = GPixel_GetB(p2);

	// modulate blend colors
	unsigned alpha = div_255(a1 * a2);
	unsigned red = div_255(r1 * r2);
	unsigned green = div_255(g1 * g2);
	unsigned blue = div_255(b1 * b2);

	return GPixel_PackARGB(alpha, red, green, blue);
}

#endif