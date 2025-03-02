"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.SceneView = SceneView;
var React = _interopRequireWildcard(require("react"));
var _reactNative = require("react-native");
var _jsxRuntime = require("react/jsx-runtime");
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function SceneView({
  children,
  navigationState,
  lazy,
  layout,
  index,
  lazyPreloadDistance,
  addEnterListener,
  style
}) {
  const [isLoading, setIsLoading] = React.useState(Math.abs(navigationState.index - index) > lazyPreloadDistance);
  if (isLoading && Math.abs(navigationState.index - index) <= lazyPreloadDistance) {
    // Always render the route when it becomes focused
    setIsLoading(false);
  }
  React.useEffect(() => {
    const handleEnter = value => {
      // If we're entering the current route, we need to load it
      if (value === index) {
        setIsLoading(prevState => {
          if (prevState) {
            return false;
          }
          return prevState;
        });
      }
    };
    let unsubscribe;
    let timer;
    if (lazy && isLoading) {
      // If lazy mode is enabled, listen to when we enter screens
      unsubscribe = addEnterListener(handleEnter);
    } else if (isLoading) {
      // If lazy mode is not enabled, render the scene with a delay if not loaded already
      // This improves the initial startup time as the scene is no longer blocking
      timer = setTimeout(() => setIsLoading(false), 0);
    }
    return () => {
      unsubscribe?.();
      clearTimeout(timer);
    };
  }, [addEnterListener, index, isLoading, lazy]);
  const focused = navigationState.index === index;
  return /*#__PURE__*/(0, _jsxRuntime.jsx)(_reactNative.View, {
    accessibilityElementsHidden: !focused,
    importantForAccessibility: focused ? 'auto' : 'no-hide-descendants',
    style: [styles.route,
    // If we don't have the layout yet, make the focused screen fill the container
    // This avoids delay before we are able to render pages side by side
    layout.width ? {
      width: layout.width
    } : focused ? _reactNative.StyleSheet.absoluteFill : null, style],
    children:
    // Only render the route only if it's either focused or layout is available
    // When layout is not available, we must not render unfocused routes
    // so that the focused route can fill the screen
    focused || layout.width ? children({
      loading: isLoading
    }) : null
  });
}
const styles = _reactNative.StyleSheet.create({
  route: {
    flex: 1,
    overflow: 'hidden'
  }
});
//# sourceMappingURL=SceneView.js.map