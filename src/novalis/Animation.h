namespace nv {
	class Animation {
	protected:
		int m_currTexIdx = 0;
		int m_firstTexIdx = 0;
		int m_lastTexIdx = 0;
		bool m_startedAnimating = false;
	public:
		Animation(int firstTexIdx, int lastTexIdx) noexcept;

		int operator()();
		void cancel() noexcept;
	};
}
