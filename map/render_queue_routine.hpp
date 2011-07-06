#pragma once

#include "../base/thread.hpp"
#include "../base/condition.hpp"
#include "../base/commands_queue.hpp"
#include "../geometry/rect2d.hpp"
#include "../geometry/screenbase.hpp"
#include "../std/list.hpp"
#include "../std/function.hpp"
#include "../yg/color.hpp"
#include "../yg/tile_cache.hpp"
#include "../yg/tiler.hpp"

class DrawerYG;

namespace threads
{
  class Condition;
}

class PaintEvent;
class WindowHandle;
class RenderQueue;

namespace yg
{
  class ResourceManager;


  namespace gl
  {
    class RenderContext;
    class FrameBuffer;
    class RenderBuffer;
    class BaseTexture;
    class RenderState;
    class RenderState;
    class Screen;
  }
}

class RenderQueueRoutine : public threads::IRoutine
{
public:

  typedef function<void(shared_ptr<PaintEvent>, ScreenBase const &, m2::RectD const &, int)> render_fn_t;
  typedef function<void()> renderCommandFinishedFn;

  /// Single tile rendering command
  struct Command
  {
    yg::Tiler::RectInfo m_rectInfo;
    shared_ptr<PaintEvent> m_paintEvent;
    render_fn_t m_renderFn;
    size_t m_seqNum;
    Command(yg::Tiler::RectInfo const & rectInfo,
            render_fn_t renderFn,
            size_t seqNum); //< paintEvent is set later after construction
  };

private:

  shared_ptr<yg::gl::RenderContext> m_renderContext;
  shared_ptr<yg::gl::FrameBuffer> m_frameBuffer;
  shared_ptr<DrawerYG> m_threadDrawer;

  threads::Mutex m_mutex;
  shared_ptr<Command> m_currentCommand;

  shared_ptr<yg::ResourceManager> m_resourceManager;

  /// A list of window handles to notify about ending rendering operations.
  list<shared_ptr<WindowHandle> > m_windowHandles;

  double m_visualScale;
  string m_skinName;
  bool m_isBenchmarking;
  unsigned m_scaleEtalonSize;
  yg::Color m_bgColor;

  size_t m_threadNum;

  list<renderCommandFinishedFn> m_renderCommandFinishedFns;

  RenderQueue * m_renderQueue;

  void callRenderCommandFinishedFns();

public:
  RenderQueueRoutine(string const & skinName,
                     bool isBenchmarking,
                     unsigned scaleEtalonSize,
                     yg::Color const & bgColor,
                     size_t threadNum,
                     RenderQueue * renderQueue);
  /// initialize GL rendering
  /// this function is called just before the thread starts.
  void initializeGL(shared_ptr<yg::gl::RenderContext> const & renderContext,
                    shared_ptr<yg::ResourceManager> const & resourceManager);
  /// This function should always be called from the main thread.
  void Cancel();
  /// Thread procedure
  void Do();
  /// invalidate all connected window handles
  void invalidate();
  /// add monitoring window
  void addWindowHandle(shared_ptr<WindowHandle> window);
  /// add model rendering command to rendering queue
  void addCommand(render_fn_t const & fn, yg::Tiler::RectInfo const & rectInfo, size_t seqNumber);
  /// add benchmark rendering command
//  void addBenchmarkCommand(render_fn_t const & fn, m2::RectD const & globalRect);
  /// set the resolution scale factor to the main thread drawer;
  void setVisualScale(double visualScale);
  /// free all available memory
  void memoryWarning();
  /// free all easily recreatable opengl resources and make sure that no opengl call will be made.
  void enterBackground();
  /// recreate all necessary opengl resources and prepare to run in foreground.
  void enterForeground();
  /// add render-command-finished callback
  void addRenderCommandFinishedFn(renderCommandFinishedFn fn);
};
