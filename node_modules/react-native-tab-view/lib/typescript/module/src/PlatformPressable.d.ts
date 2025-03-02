import * as React from 'react';
import { type PressableProps } from 'react-native';
export type Props = PressableProps & {
    children: React.ReactNode;
    pressColor?: string;
    pressOpacity?: number;
    href?: string;
};
/**
 * PlatformPressable provides an abstraction on top of Pressable to handle platform differences.
 */
export declare function PlatformPressable({ disabled, android_ripple, pressColor, pressOpacity, style, onPress, ...rest }: Props): import("react/jsx-runtime").JSX.Element;
//# sourceMappingURL=PlatformPressable.d.ts.map