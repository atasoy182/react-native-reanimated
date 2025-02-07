#include "UIRuntimeDecorator.h"
#include "ReanimatedJSIUtils.h"

namespace reanimated {

void UIRuntimeDecorator::decorate(
    jsi::Runtime &uiRuntime,
#ifdef RCT_NEW_ARCH_ENABLED
    const RemoveFromPropsRegistryFunction removeFromPropsRegistry,
#else
    const ScrollToFunction scrollTo,
#endif
    const UpdatePropsFunction updateProps,
    const MeasureFunction measure,
    const DispatchCommandFunction dispatchCommand,
    const RequestAnimationFrameFunction requestAnimationFrame,
    const TimeProviderFunction getCurrentTime,
    const SetGestureStateFunction setGestureState,
    const ProgressLayoutAnimationFunction progressLayoutAnimation,
    const EndLayoutAnimationFunction endLayoutAnimation,
    const MaybeFlushUIUpdatesQueueFunction maybeFlushUIUpdatesQueue) {
  uiRuntime.global().setProperty(uiRuntime, "_UI", true);

#ifdef RCT_NEW_ARCH_ENABLED
  jsi_utils::installJsiFunction(uiRuntime, "_updatePropsFabric", updateProps);
  jsi_utils::installJsiFunction(
      uiRuntime, "_removeFromPropsRegistry", removeFromPropsRegistry);
  jsi_utils::installJsiFunction(
      uiRuntime, "_dispatchCommandFabric", dispatchCommand);
  jsi_utils::installJsiFunction(uiRuntime, "_measureFabric", measure);
#else
  jsi_utils::installJsiFunction(uiRuntime, "_updatePropsPaper", updateProps);
  jsi_utils::installJsiFunction(
      uiRuntime, "_dispatchCommandPaper", dispatchCommand);
  jsi_utils::installJsiFunction(uiRuntime, "_scrollToPaper", scrollTo);
  jsi_utils::installJsiFunction(
      uiRuntime,
      "_measurePaper",
      [measure](jsi::Runtime &rt, int viewTag) -> jsi::Value {
        auto result = measure(viewTag);
        jsi::Object resultObject(rt);
        for (const auto &item : result) {
          resultObject.setProperty(rt, item.first.c_str(), item.second);
        }
        return resultObject;
      });
#endif // RCT_NEW_ARCH_ENABLED

  jsi_utils::installJsiFunction(
      uiRuntime, "requestAnimationFrame", requestAnimationFrame);

  auto performanceNow = [getCurrentTime](
                            jsi::Runtime &,
                            const jsi::Value &,
                            const jsi::Value *,
                            size_t) -> jsi::Value {
    return jsi::Value(getCurrentTime());
  };
  jsi::Object performance(uiRuntime);
  performance.setProperty(
      uiRuntime,
      "now",
      jsi::Function::createFromHostFunction(
          uiRuntime,
          jsi::PropNameID::forAscii(uiRuntime, "now"),
          0,
          performanceNow));
  uiRuntime.global().setProperty(uiRuntime, "performance", performance);

  jsi_utils::installJsiFunction(
      uiRuntime, "_notifyAboutProgress", progressLayoutAnimation);
  jsi_utils::installJsiFunction(
      uiRuntime, "_notifyAboutEnd", endLayoutAnimation);

  jsi_utils::installJsiFunction(uiRuntime, "_setGestureState", setGestureState);
  jsi_utils::installJsiFunction(
      uiRuntime, "_maybeFlushUIUpdatesQueue", maybeFlushUIUpdatesQueue);
}

} // namespace reanimated
