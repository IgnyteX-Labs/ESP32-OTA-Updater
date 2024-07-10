#ifndef SEMANTIC_VERSION_H_
#define SEMANTIC_VERSION_H_

#include <stdio.h>
#include <string.h>

/**
 * @file SemanticVersion.h
 * @brief Contains the declaration of the Version class.
 */

/**
 * @class Version
 * @brief Represents a semantic version.
 */
class Version
{
private:
    int major; /**< The major version number. */
    int minor; /**< The minor version number. */
    int patch; /**< The patch version number. */

public:
    /**
     * @brief Constructs a Version object with the specified major, minor, and patch version numbers.
     * @param major The major version number.
     * @param minor The minor version number.
     * @param patch The patch version number.
     */
    Version(int major, int minor, int patch) : major(major), minor(minor), patch(patch) {}

    /**
     * @brief Constructs a Version object from a string representation of the version.
     * @param version The string representation of the version.
     *                The format should be "vX.Y.Z" or "X.Y.Z", where X, Y, and Z are integers.
     */
    Version(const char *version) : major(0), minor(0), patch(0)
    {
        if (version != nullptr && strlen(version) > 0)
        {
            if (version[0] == 'v')
            {
                sscanf(version, "v%d.%d.%d", &major, &minor, &patch);
            }
            else
            {
                sscanf(version, "%d.%d.%d", &major, &minor, &patch);
            }
        }
    }

    /**
     * @brief Gets the major version number.
     * @return The major version number.
     */
    int getMajor() const
    {
        return major;
    }

    /**
     * @brief Gets the minor version number.
     * @return The minor version number.
     */
    int getMinor() const
    {
        return minor;
    }

    /**
     * @brief Gets the patch version number.
     * @return The patch version number.
     */
    int getPatch() const
    {
        return patch;
    }

    /**
     * @brief Checks if this Version object is equal to another Version object.
     * @param other The other Version object to compare with.
     * @return True if the two Version objects are equal, false otherwise.
     */
    bool operator==(const Version &other) const
    {
        return (major == other.major && minor == other.minor && patch == other.patch);
    }

    /**
     * @brief Checks if this Version object is not equal to another Version object.
     * @param other The other Version object to compare with.
     * @return True if the two Version objects are not equal, false otherwise.
     */
    bool operator!=(const Version &other) const
    {
        return !(*this == other);
    }

    /**
     * @brief Checks if this Version object is less than another Version object.
     * @param other The other Version object to compare with.
     * @return True if this Version object is less than the other Version object, false otherwise.
     */
    bool operator<(const Version &other) const
    {
        if (major < other.major)
            return true;
        if (major > other.major)
            return false;
        if (minor < other.minor)
            return true;
        if (minor > other.minor)
            return false;
        return patch < other.patch;
    }

    /**
     * @brief Checks if this Version object is greater than another Version object.
     * @param other The other Version object to compare with.
     * @return True if this Version object is greater than the other Version object, false otherwise.
     */
    bool operator>(const Version &other) const
    {
        return other < *this;
    }

    /**
     * @brief Checks if this Version object is less than or equal to another Version object.
     * @param other The other Version object to compare with.
     * @return True if this Version object is less than or equal to the other Version object, false otherwise.
     */
    bool operator<=(const Version &other) const
    {
        return !(other < *this);
    }

    /**
     * @brief Checks if this Version object is greater than or equal to another Version object.
     * @param other The other Version object to compare with.
     * @return True if this Version object is greater than or equal to the other Version object, false otherwise.
     */
    bool operator>=(const Version &other) const
    {
        return !(*this < other);
    }
};

#endif // SEMANTIC_VERSION_H_