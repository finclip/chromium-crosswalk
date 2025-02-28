// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_TASK_MANAGEMENT_PROVIDERS_WEB_CONTENTS_RENDERER_TASK_H_
#define CHROME_BROWSER_TASK_MANAGEMENT_PROVIDERS_WEB_CONTENTS_RENDERER_TASK_H_

#include <stdint.h>

#include <memory>

#include "base/macros.h"
#include "chrome/browser/task_management/providers/task.h"
#include "components/favicon/core/favicon_driver_observer.h"
#include "content/public/browser/navigation_entry.h"

class ProcessResourceUsage;

namespace content {
class RenderProcessHost;
class WebContents;
}  // namespace content

namespace task_management {

// Defines an abstract base class for various types of renderer process tasks
// such as background contents, tab contents, ... etc.
class RendererTask : public Task,
                     public favicon::FaviconDriverObserver {
 public:
  RendererTask(const base::string16& title,
               const gfx::ImageSkia* icon,
               content::WebContents* web_contents,
               content::RenderProcessHost* render_process_host);
  ~RendererTask() override;

  // An abstract method that will be called when the event
  // WebContentsObserver::DidNavigateMainFrame() occurs. This gives the
  // freedom to concrete tasks to adjust the title however they need to before
  // they set it.
  virtual void UpdateTitle() = 0;

  // An abstract method that will be called when the event
  // FaviconDriverObserver::OnFaviconUpdated() occurs, so that concrete tasks
  // can update their favicons.
  virtual void UpdateFavicon() = 0;

  // An overridable method that will be called when the event
  // WebContentsObserver::DidNavigateMainFrame() occurs, so that we can update
  // their Rappor sample name when a navigation takes place.
  virtual void UpdateRapporSampleName();

  // task_management::Task:
  void Activate() override;
  void Refresh(const base::TimeDelta& update_interval,
               int64_t refresh_flags) override;
  Type GetType() const override;
  int GetChildProcessUniqueID() const override;
  void GetTerminationStatus(base::TerminationStatus* out_status,
                            int* out_error_code) const override;
  base::string16 GetProfileName() const override;
  int GetTabId() const override;
  int64_t GetV8MemoryAllocated() const override;
  int64_t GetV8MemoryUsed() const override;
  bool ReportsWebCacheStats() const override;
  blink::WebCache::ResourceTypeStats GetWebCacheStats() const override;

  // favicon::FaviconDriverObserver:
  void OnFaviconUpdated(favicon::FaviconDriver* driver,
                        NotificationIconType notification_icon_type,
                        const GURL& icon_url,
                        bool icon_url_changed,
                        const gfx::Image& image) override;

  void set_termination_status(base::TerminationStatus status) {
    termination_status_ = status;
  }

  void set_termination_error_code(int error_code) {
    termination_error_code_ = error_code;
  }

 protected:
  // Returns the title of the given |web_contents|.
  static base::string16 GetTitleFromWebContents(
      content::WebContents* web_contents);

  // Returns the favicon of the given |web_contents| if any, and returns
  // |nullptr| otherwise.
  static const gfx::ImageSkia* GetFaviconFromWebContents(
      content::WebContents* web_contents);

  // Prefixes the given renderer |title| with the appropriate string based on
  // whether it's an app, an extension, incognito or a background page or
  // contents.
  static const base::string16 PrefixRendererTitle(const base::string16& title,
                                                  bool is_app,
                                                  bool is_extension,
                                                  bool is_incognito,
                                                  bool is_background);

  content::WebContents* web_contents() const { return web_contents_; }

 private:
  // The WebContents of the task this object represents.
  content::WebContents* web_contents_;

  // The render process host of the task this object represents.
  content::RenderProcessHost* render_process_host_;

  // The Mojo service wrapper that will provide us with the V8 memory usage and
  // the WebCache resource stats of the render process represented by this
  // object.
  std::unique_ptr<ProcessResourceUsage> renderer_resources_sampler_;

  // The unique ID of the RenderProcessHost.
  const int render_process_id_;

  // The allocated and used V8 memory (in bytes).
  int64_t v8_memory_allocated_;
  int64_t v8_memory_used_;

  // The WebKit resource cache statistics for this renderer.
  blink::WebCache::ResourceTypeStats webcache_stats_;

  // The profile name associated with the browser context of the render view
  // host.
  const base::string16 profile_name_;

  base::TerminationStatus termination_status_;
  int termination_error_code_;

  DISALLOW_COPY_AND_ASSIGN(RendererTask);
};

}  // namespace task_management

#endif  // CHROME_BROWSER_TASK_MANAGEMENT_PROVIDERS_WEB_CONTENTS_RENDERER_TASK_H_
