static class MouseController
{
public:
	MouseController();
	static void mouseFunc(int button, int state, int x, int y);
	void mouseDrag(int x, int y);
	void mouseClick(int button, int state, int x, int y);
	void motionFunc(int x, int y);
};
