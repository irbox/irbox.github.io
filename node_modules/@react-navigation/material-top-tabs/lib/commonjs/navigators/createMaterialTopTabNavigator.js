"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.createMaterialTopTabNavigator = createMaterialTopTabNavigator;
var _native = require("@react-navigation/native");
var _MaterialTopTabView = require("../views/MaterialTopTabView.js");
var _jsxRuntime = require("react/jsx-runtime");
function MaterialTopTabNavigator({
  id,
  initialRouteName,
  backBehavior,
  children,
  layout,
  screenListeners,
  screenOptions,
  screenLayout,
  UNSTABLE_getStateForRouteNamesChange,
  ...rest
}) {
  const {
    state,
    descriptors,
    navigation,
    NavigationContent
  } = (0, _native.useNavigationBuilder)(_native.TabRouter, {
    id,
    initialRouteName,
    backBehavior,
    children,
    layout,
    screenListeners,
    screenOptions,
    screenLayout,
    UNSTABLE_getStateForRouteNamesChange
  });
  return /*#__PURE__*/(0, _jsxRuntime.jsx)(NavigationContent, {
    children: /*#__PURE__*/(0, _jsxRuntime.jsx)(_MaterialTopTabView.MaterialTopTabView, {
      ...rest,
      state: state,
      navigation: navigation,
      descriptors: descriptors
    })
  });
}
function createMaterialTopTabNavigator(config) {
  return (0, _native.createNavigatorFactory)(MaterialTopTabNavigator)(config);
}
//# sourceMappingURL=createMaterialTopTabNavigator.js.map