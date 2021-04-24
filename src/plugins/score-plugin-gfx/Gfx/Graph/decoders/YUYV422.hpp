#pragma once
#include <Gfx/Graph/decoders/GPUVideoDecoder.hpp>

#include <Gfx/Qt5CompatPush>
struct YUYV422Decoder : GPUVideoDecoder
{
static const constexpr auto filter = R"_(#version 450

layout(std140, binding = 0) uniform buf {
mat4 clipSpaceCorrMatrix;
vec2 texcoordAdjust;
} tbuf;

layout(binding=3) uniform sampler2D u_tex;

layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;

// See https://gist.github.com/roxlu/7872352
const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);
const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);
const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);
const vec3 offset = vec3(-0.0625, -0.5, -0.5);

void main() {
  vec2 texcoord = vec2(v_texcoord.x, tbuf.texcoordAdjust.y + tbuf.texcoordAdjust.x * v_texcoord.y);
  vec3 tc = texture(u_tex, texcoord).rgb;
  vec3 yuv = vec3(tc.g, tc.b, tc.r);
  yuv += offset;
  fragColor.r = dot(yuv, R_cf);
  fragColor.g = dot(yuv, G_cf);
  fragColor.b = dot(yuv, B_cf);
  fragColor.a = 1.0;
}
)_";

  YUYV422Decoder(NodeModel& n, video_decoder& d): node{n}, decoder{d} { }
  NodeModel& node;
  video_decoder& decoder;
  void init(Renderer& r, RenderedNode& rendered) override
  {
    auto& rhi = *r.state.rhi;

    std::tie(node.m_vertexS, node.m_fragmentS) = makeShaders(node.mesh().defaultVertexShader(), filter);

    const auto w = decoder.width, h = decoder.height;
    // Y
    {
      auto tex = rhi.newTexture(QRhiTexture::RGBA8, {w/2, h}, 1, QRhiTexture::Flag{});
      tex->create();

      auto sampler = rhi.newSampler(
          QRhiSampler::Linear,
          QRhiSampler::Linear,
          QRhiSampler::None,
          QRhiSampler::ClampToEdge,
          QRhiSampler::ClampToEdge);
      sampler->create();
      rendered.m_samplers.push_back({sampler, tex});
    }
  }

  void exec(Renderer&, RenderedNode& rendered, QRhiResourceUpdateBatch& res, AVFrame& frame) override
  {
    setYPixels(rendered, res, frame.data[0], frame.linesize[0]);
  }

  void release(Renderer&, RenderedNode& n) override
  {
    for (auto [sampler, tex] : n.m_samplers)
      tex->deleteLater();
  }

  void setYPixels(RenderedNode& rendered, QRhiResourceUpdateBatch& res, uint8_t* pixels, int stride) const noexcept
  {
    const auto w = decoder.width, h = decoder.height;
    auto y_tex = rendered.m_samplers[0].texture;

    QRhiTextureUploadEntry entry{0, 0, createTextureUpload(pixels, w, h, 2, stride)};

    QRhiTextureUploadDescription desc{entry};
    res.uploadTexture(y_tex, desc);
  }

};

struct UYVY422Decoder : GPUVideoDecoder
{
static const constexpr auto filter = R"_(#version 450

layout(std140, binding = 0) uniform buf {
mat4 clipSpaceCorrMatrix;
vec2 texcoordAdjust;
vec2 RENDERSIZE;
} tbuf;

layout(binding=3) uniform sampler2D u_tex;

layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;

const mat4 bt601 = mat4(
                    1.164f,  1.164f,  1.164f,   0.0f,
                    0.000f, -0.392f,  2.017f,   0.0f,
                    1.596f, -0.813f,  0.000f,   0.0f,
                    -0.8708f, 0.5296f, -1.081f, 1.0f);
const mat4 bt709 = mat4(
                    1.164f,  1.164f,  1.164f,   0.0f,
                    0.000f, -0.534f,  2.115,    0.0f,
                    1.793f, -0.213f,  0.000f,   0.0f,
                    -0.5727f, 0.3007f, -1.1302, 1.0f);

// This code was adapted from QtMultimedia
void main() {
  vec2 texcoord = vec2(v_texcoord.x, tbuf.texcoordAdjust.y + tbuf.texcoordAdjust.x * v_texcoord.y);

   // For U0 Y0 V0 Y1 macropixel, lookup Y0 or Y1 based on whether
   // the original texture x coord is even or odd.
   vec4 uyvy = texture(u_tex, texcoord);
   float Y;
   if (fract(floor(texcoord.x * tbuf.RENDERSIZE.x + 0.5) / 2.0) > 0.0)
       Y = uyvy.a;       // odd so choose Y1
   else
       Y = uyvy.g;       // even so choose Y0
   float Cb = uyvy.r;
   float Cr = uyvy.b;

   vec4 color = vec4(Y, Cb, Cr, 1.0);
   fragColor = bt601 * color;
}
)_";

  UYVY422Decoder(NodeModel& n, video_decoder& d): node{n}, decoder{d} { }
  NodeModel& node;
  video_decoder& decoder;
  void init(Renderer& r, RenderedNode& rendered) override
  {
    auto& rhi = *r.state.rhi;

    std::tie(node.m_vertexS, node.m_fragmentS) = makeShaders(node.mesh().defaultVertexShader(), filter);

    const auto w = decoder.width, h = decoder.height;
    // Y
    {
      auto tex = rhi.newTexture(QRhiTexture::RGBA8, {w/2, h}, 1, QRhiTexture::Flag{});
      tex->create();

      auto sampler = rhi.newSampler(
          QRhiSampler::Linear,
          QRhiSampler::Linear,
          QRhiSampler::None,
          QRhiSampler::ClampToEdge,
          QRhiSampler::ClampToEdge);
      sampler->create();
      rendered.m_samplers.push_back({sampler, tex});
    }
  }

  void exec(Renderer&, RenderedNode& rendered, QRhiResourceUpdateBatch& res, AVFrame& frame) override
  {
    setYPixels(rendered, res, frame.data[0], frame.linesize[0]);
  }

  void release(Renderer&, RenderedNode& n) override
  {
    for (auto [sampler, tex] : n.m_samplers)
      tex->deleteLater();
  }

  void setYPixels(RenderedNode& rendered, QRhiResourceUpdateBatch& res, uint8_t* pixels, int stride) const noexcept
  {
    const auto w = decoder.width, h = decoder.height;
    auto y_tex = rendered.m_samplers[0].texture;

    QRhiTextureUploadEntry entry{0, 0, createTextureUpload(pixels, w, h, 2, stride)};

    QRhiTextureUploadDescription desc{entry};
    res.uploadTexture(y_tex, desc);
  }

};

#include <Gfx/Qt5CompatPop>
