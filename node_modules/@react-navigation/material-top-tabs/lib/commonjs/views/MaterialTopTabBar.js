"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.MaterialTopTabBar = MaterialTopTabBar;
var _elements = require("@react-navigation/elements");
var _native = require("@react-navigation/native");
var _color = _interopRequireDefault(require("color"));
var _reactNative = require("react-native");
var _reactNativeTabView = require("react-native-tab-view");
var _jsxRuntime = require("react/jsx-runtime");
function _interopRequireDefault(e) { return e && e.__esModule ? e : { default: e }; }
const renderLabelDefault = ({
  color,
  labelText,
  style,
  allowFontScaling
}) => {
  return /*#__PURE__*/(0, _jsxRuntime.jsx)(_elements.Text, {
    style: [{
      color
    }, styles.label, style],
    allowFontScaling: allowFontScaling,
    children: labelText
  });
};
function MaterialTopTabBar({
  state,
  navigation,
  descriptors,
  ...rest
}) {
  const {
    colors
  } = (0, _native.useTheme)();
  const {
    direction
  } = (0, _native.useLocale)();
  const {
    buildHref
  } = (0, _native.useLinkBuilder)();
  const focusedOptions = descriptors[state.routes[state.index].key].options;
  const activeColor = focusedOptions.tabBarActiveTintColor ?? colors.text;
  const inactiveColor = focusedOptions.tabBarInactiveTintColor ?? (0, _color.default)(activeColor).alpha(0.5).rgb().string();
  const tabBarOptions = Object.fromEntries(state.routes.map(route => {
    const {
      options
    } = descriptors[route.key];
    const {
      title,
      tabBarLabel,
      tabBarButtonTestID,
      tabBarAccessibilityLabel,
      tabBarBadge,
      tabBarShowIcon,
      tabBarShowLabel,
      tabBarIcon,
      tabBarAllowFontScaling,
      tabBarLabelStyle
    } = options;
    return [route.key, {
      href: buildHref(route.name, route.params),
      testID: tabBarButtonTestID,
      accessibilityLabel: tabBarAccessibilityLabel,
      badge: tabBarBadge,
      icon: tabBarShowIcon === false ? undefined : tabBarIcon,
      label: tabBarShowLabel === false ? undefined : typeof tabBarLabel === 'function' ? ({
        labelText,
        color
      }) => tabBarLabel({
        focused: state.routes[state.index].key === route.key,
        color,
        children: labelText ?? route.name
      }) : renderLabelDefault,
      labelAllowFontScaling: tabBarAllowFontScaling,
      labelStyle: tabBarLabelStyle,
      labelText: options.tabBarShowLabel === false ? undefined : typeof tabBarLabel === 'string' ? tabBarLabel : title !== undefined ? title : route.name
    }];
  }));
  return /*#__PURE__*/(0, _jsxRuntime.jsx)(_reactNativeTabView.TabBar, {
    ...rest,
    navigationState: state,
    options: tabBarOptions,
    direction: direction,
    scrollEnabled: focusedOptions.tabBarScrollEnabled,
    bounces: focusedOptions.tabBarBounces,
    activeColor: activeColor,
    inactiveColor: inactiveColor,
    pressColor: focusedOptions.tabBarPressColor,
    pressOpacity: focusedOptions.tabBarPressOpacity,
    tabStyle: focusedOptions.tabBarItemStyle,
    indicatorStyle: [{
      backgroundColor: colors.primary
    }, focusedOptions.tabBarIndicatorStyle],
    gap: focusedOptions.tabBarGap,
    android_ripple: focusedOptions.tabBarAndroidRipple,
    indicatorContainerStyle: focusedOptions.tabBarIndicatorContainerStyle,
    contentContainerStyle: focusedOptions.tabBarContentContainerStyle,
    style: [{
      backgroundColor: colors.card
    }, focusedOptions.tabBarStyle],
    onTabPress: ({
      route,
      preventDefault
    }) => {
      const event = navigation.emit({
        type: 'tabPress',
        target: route.key,
        canPreventDefault: true
      });
      if (event.defaultPrevented) {
        preventDefault();
      }
    },
    onTabLongPress: ({
      route
    }) => navigation.emit({
      type: 'tabLongPress',
      target: route.key
    }),
    renderIndicator: ({
      navigationState: state,
      ...rest
    }) => {
      return focusedOptions.tabBarIndicator ? focusedOptions.tabBarIndicator({
        state: state,
        ...rest
      }) : /*#__PURE__*/(0, _jsxRuntime.jsx)(_reactNativeTabView.TabBarIndicator, {
        navigationState: state,
        ...rest
      });
    }
  });
}
const styles = _reactNative.StyleSheet.create({
  label: {
    textAlign: 'center',
    fontSize: 14,
    margin: 4,
    backgroundColor: 'transparent'
  }
});
//# sourceMappingURL=MaterialTopTabBar.js.map