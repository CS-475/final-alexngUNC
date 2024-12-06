struct Edge {
	float m;
	float b;
	int top;
	int bottom;
	int w; // +1 = up, -1 = down
	float x; // for sorting leftmost x value in ray cast

	inline bool valid(int y) {
		return top <= y && y < bottom;
	}

	inline float computeX(int y) {
		return m * (y + 0.5f) + b;
	}
};