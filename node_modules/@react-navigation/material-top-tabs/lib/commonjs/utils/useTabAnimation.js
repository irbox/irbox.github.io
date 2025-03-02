"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.useTabAnimation = useTabAnimation;
var React = _interopRequireWildcard(require("react"));
var _TabAnimationContext = require("./TabAnimationContext.js");
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function useTabAnimation() {
  const animation = React.useContext(_TabAnimationContext.TabAnimationContext);
  if (animation === undefined) {
    throw new Error("Couldn't find values for tab animation. Are you inside a screen in Material Top Tab navigator?");
  }
  return animation;
}
//# sourceMappingURL=useTabAnimation.js.map