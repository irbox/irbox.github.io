/**
 * @format
 */
import { Platform, NativeModules } from 'react-native';
const Profile = NativeModules.FBProfile;
/**
 * Represents an immutable Facebook profile
 * This class provides a global "currentProfile" instance to more easily add social context to your application.
 */
class FBProfile {
  /**
   * The user id
   */

  /**
   * The user's email.
   * IMPORTANT: This field will only be populated if your user has granted your application the 'email' permission.
   */

  /**
   * The user's complete name
   */

  /**
   * The user's first name
   */

  /**
   * The user's last name
   */

  /**
   * The user's middle name
   */

  /**
   * A URL to the user's profile.
   * IMPORTANT: This field will only be populated if your user has granted your application the 'user_link' permission
   */

  /**
   * A URL to use for fetching a user's profile image.
   */

  /**
   * The last time the profile data was fetched.
   */

  /**
   * A list of identifiers of the user’s friends.
   * IMPORTANT: This field will only be populated if your user has granted your application the 'user_friends' permission and
   * limited login flow is used on iOS
   */

  /**
   * The user’s birthday.
   * IMPORTANT: This field will only be populated if your user has granted your application the 'user_birthday' permission and
   * limited login flow is used on iOS
   */

  /**
   * The user’s age range.
   * IMPORTANT: This field will only be populated if your user has granted your application the 'user_age_range' permission and
   * limited login flow is used on iOS
   */

  /**
   * The user’s hometown.
   * IMPORTANT: This field will only be populated if your user has granted your application the 'user_hometown' permission and
   * limited login flow is used on iOS
   */

  /**
   * The user’s location.
   * IMPORTANT: This field will only be populated if your user has granted your application the 'user_location' permission and
   * limited login flow is used on iOS
   */

  /**
   * The user’s gender.
   * IMPORTANT: This field will only be populated if your user has granted your application the 'user_gender' permission and
   * limited login flow is used on iOS
   */

  /**
   * The user’s granted permissions.
   */

  constructor(profileMap) {
    this.firstName = profileMap.firstName;
    this.lastName = profileMap.lastName;
    this.middleName = profileMap.middleName;
    this.linkURL = profileMap.linkURL;
    this.imageURL = profileMap.imageURL;
    this.userID = profileMap.userID;
    if (Platform.OS !== 'android') {
      this.email = profileMap.email;
    }
    this.name = profileMap.name;
    this.refreshDate = profileMap.refreshDate ? new Date(profileMap.refreshDate) : null;
    this.friendIDs = profileMap.friendIDs;
    this.birthday = profileMap.birthday ? new Date(profileMap.birthday) : null;
    this.ageRange = profileMap.ageRange;
    this.hometown = profileMap.hometown;
    this.location = profileMap.location;
    this.gender = profileMap.gender;
    this.permissions = profileMap.permissions;
    Object.freeze(this);
  }

  /**
   * Getter the current logged profile
   */
  static getCurrentProfile() {
    return new Promise(resolve => {
      Profile.getCurrentProfile(profileMap => {
        if (profileMap) {
          resolve(new FBProfile(profileMap));
        } else {
          resolve(null);
        }
      });
    });
  }
}
export default FBProfile;
//# sourceMappingURL=FBProfile.js.map