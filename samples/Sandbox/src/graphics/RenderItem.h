/*
	Renderable class:
	
	Abstract class that encapsulates a set of draw command and some data.
*/

#include <tsgraphics/GraphicsContext.h>

namespace ts
{
	class RenderItem
	{
	private:
		
		GraphicsContext* m_context;
		HDrawCmd m_draw;
		
	protected:
		
		virtual void onDraw(CommandQueue* queue) = 0;
		
	public:
		
		RenderItem(GraphicsContext* context);
		~RenderItem();
		
		void draw();
	};
}
