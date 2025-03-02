declare const _default: {
    /**
     * For iOS only, get AdvertiserTrackingEnabled status.
     * @platform ios
     */
    getAdvertiserTrackingEnabled(): Promise<boolean>;
    /**
     * For iOS only, set AdvertiserTrackingEnabled status, only works in iOS 14 and above.
     * @platform ios
     */
    setAdvertiserTrackingEnabled(ATE: boolean): Promise<boolean>;
    /**
     * Set data processing options
     */
    setDataProcessingOptions(options: Array<string>, ...args: Array<number>): void;
    /**
     * Initialize the sdk
     */
    initializeSDK(): void;
    /**
     * Set app id
     */
    setAppID(appID: string): void;
    /**
     * Set clientToken
     */
    setClientToken(clientToken: string): void;
    /**
     * Sets the Facebook application name for the current app.
     */
    setAppName(appName: string): void;
    /**
     * Sets the Graph API version to use when making Graph requests.
     */
    setGraphAPIVersion(version: string): void;
    /**
     * Sets whether Facebook SDK should log app events. App events involve eg. app installs,
     * app launches etc.
     */
    setAutoLogAppEventsEnabled(enabled: boolean): void;
    /**
     * Whether the Facebook SDK should collect advertiser ID properties, like the Apple IDFA
     * and Android Advertising ID, automatically. Advertiser IDs let you identify and target
     * specific customers.
     */
    setAdvertiserIDCollectionEnabled(enabled: boolean): void;
};
export default _default;
//# sourceMappingURL=FBSettings.d.ts.map