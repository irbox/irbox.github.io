"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = void 0;
var _reactNative = require("react-native");
/**
 * @format
 */

const AuthenticationToken = _reactNative.NativeModules.FBAuthenticationToken;
/**
 * Represents an immutable access token for using Facebook services.
 */
class FBAuthenticationToken {
  /**
     The raw token string from the authentication response
    */

  /**
     The nonce from the decoded authentication response
    */

  /**
    The graph domain where the user is authenticated.
   */

  constructor(tokenMap) {
    this.authenticationToken = tokenMap.authenticationToken;
    this.nonce = tokenMap.nonce;
    this.graphDomain = tokenMap.graphDomain;
    Object.freeze(this);
  }

  /**
   * Getter for the authentication token
   */
  static getAuthenticationTokenIOS() {
    if (_reactNative.Platform.OS === 'android') {
      return Promise.resolve(null);
    }
    return new Promise(resolve => {
      AuthenticationToken.getAuthenticationToken(tokenMap => {
        if (tokenMap) {
          resolve(new FBAuthenticationToken(tokenMap));
        } else {
          resolve(null);
        }
      });
    });
  }
}
var _default = exports.default = FBAuthenticationToken;
//# sourceMappingURL=FBAuthenticationToken.js.map