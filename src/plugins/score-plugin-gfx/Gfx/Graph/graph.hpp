#pragma once
#include "node.hpp"
#include "renderer.hpp"

#include <ossia/detail/algorithms.hpp>

#include <score_plugin_gfx_export.h>
struct OutputNode;
class Window;
struct SCORE_PLUGIN_GFX_EXPORT Graph
{
  std::vector<score::gfx::Node*> nodes;
  std::vector<Edge*> edges;

  void addNode(score::gfx::Node* n) { nodes.push_back(n); }

  void removeNode(score::gfx::Node* n)
  {
    if (auto it = ossia::find(nodes, n); it != nodes.end())
    {
      nodes.erase(it);
    }
  }

  void setVSyncCallback(std::function<void()>);

  std::shared_ptr<Renderer> createRenderer(OutputNode*, RenderState state);

  void setupOutputs(GraphicsApi graphicsApi);

  void relinkGraph();

  ~Graph();

  std::vector<OutputNode*> outputs;

private:
  std::vector<std::shared_ptr<Renderer>> renderers;

  std::vector<std::shared_ptr<Window>> unused_windows;
  std::function<void()> vsync_callback;
};

#if QT_CONFIG(vulkan)
class QVulkanInstance;
QVulkanInstance* staticVulkanInstance();
#endif
