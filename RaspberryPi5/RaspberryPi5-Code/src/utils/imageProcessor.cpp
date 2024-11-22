#include "imageProcessor.h"

std::vector<cv::Point> getCoordinates(const cv::Mat &mask) {
    std::vector<cv::Point> coordinates;
    cv::findNonZero(mask, coordinates);
    return coordinates;
}

int getAverageX(const std::vector<cv::Point> &coordinates) {
    if (coordinates.empty()) return -1;
    int sum = 0;
    for (const auto &pt : coordinates) sum += pt.x;
    return sum / coordinates.size();
}

int getAverageY(const std::vector<cv::Point> &coordinates) {
    if (coordinates.empty()) return -1;
    int sum = 0;
    for (const auto &pt : coordinates) sum += pt.y;
    return sum / coordinates.size();
}

std::tuple<cv::Point, double, cv::Point> getCentroidAndArea(const std::vector<cv::Point> &contour) {
    cv::Moments M = cv::moments(contour, true);
    if (M.m00 == 0) return {cv::Point(-1, -1), 0, cv::Point(-1, -1)};
    cv::Point centroid(static_cast<int>(M.m10 / M.m00), static_cast<int>(M.m01 / M.m00));
    double area = cv::contourArea(contour);
    cv::Point lowestPoint = *std::max_element(contour.begin(), contour.end(), [](cv::Point a, cv::Point b) { return a.y < b.y; });
    return {centroid, area, lowestPoint};
}

ImageProcessingResult processImage(const cv::Mat &image) {
    cv::Mat hsvImage;
    cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

    cv::Mat maskBlue, maskOrange, maskRed1, maskRed2, maskRed, maskGreen, maskPink;
    cv::inRange(hsvImage, lowerBlueLine, upperBlueLine, maskBlue);
    cv::inRange(hsvImage, lowerOrangeLine, upperOrangeLine, maskOrange);
    cv::inRange(hsvImage, lowerRed1Light, upperRed1Light, maskRed1);
    cv::inRange(hsvImage, lowerRed2Light, upperRed2Light, maskRed2);
    maskRed = maskRed1 | maskRed2;
    cv::inRange(hsvImage, lowerGreenLight, upperGreenLight, maskGreen);
    cv::inRange(hsvImage, lowerPinkLight, upperPinkLight, maskPink);

    std::vector<std::vector<cv::Point>> contoursBlue, contoursOrange, contoursRed, contoursGreen;
    cv::findContours(maskBlue, contoursBlue, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(maskOrange, contoursOrange, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(maskRed, contoursRed, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(maskGreen, contoursGreen, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Point> blueCoordinates = getCoordinates(maskBlue);
    std::vector<cv::Point> orangeCoordinates = getCoordinates(maskOrange);
    std::vector<cv::Point> pinkCoordinates = getCoordinates(maskPink);

    int blueLineY = getAverageY(blueCoordinates);
    int blueLineSize = blueCoordinates.size();
    int orangeLineY = getAverageY(orangeCoordinates);
    int orangeLineSize = orangeCoordinates.size();
    int pinkX = getAverageX(pinkCoordinates);
    int pinkY = getAverageY(pinkCoordinates);
    int pinkSize = pinkCoordinates.size();

    // Placeholder values for the closest block
    int closestBlockX = -1, closestBlockY = -1, closestBlockLowestY = -1, closestBlockSize = -1;
    std::string closestBlockColor = "None";

    return {blueLineY, blueLineSize, orangeLineY, orangeLineSize,
            closestBlockX, closestBlockY, closestBlockLowestY,
            closestBlockSize, closestBlockColor, pinkX, pinkY, pinkSize};
}

cv::Mat filterAllColors(const cv::Mat &image) {
    cv::Mat hsvImage, maskBlue, maskOrange, maskRed1, maskRed2, maskRed, maskGreen, maskPink;
    cv::Mat filledBlue, filledOrange, filledRed, filledGreen, filledPink, finalImage;

    // Convert the image to HSV color space
    cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

    // Create masks for each color range
    cv::inRange(hsvImage, lowerBlueLine, upperBlueLine, maskBlue);
    cv::inRange(hsvImage, lowerOrangeLine, upperOrangeLine, maskOrange);
    cv::inRange(hsvImage, lowerRed1Light, upperRed1Light, maskRed1);
    cv::inRange(hsvImage, lowerRed2Light, upperRed2Light, maskRed2);
    cv::inRange(hsvImage, lowerGreenLight, upperGreenLight, maskGreen);
    cv::inRange(hsvImage, lowerPinkLight, upperPinkLight, maskPink);

    // Merge two red masks
    maskRed = maskRed1 | maskRed2;

    // Create colored images for each mask
    filledBlue = cv::Mat::zeros(image.size(), image.type());
    filledBlue.setTo(cv::Scalar(255, 0, 0), maskBlue); // Blue (BGR format)

    filledOrange = cv::Mat::zeros(image.size(), image.type());
    filledOrange.setTo(cv::Scalar(0, 165, 255), maskOrange); // Orange (BGR format)

    filledRed = cv::Mat::zeros(image.size(), image.type());
    filledRed.setTo(cv::Scalar(0, 0, 255), maskRed); // Red (BGR format)

    filledGreen = cv::Mat::zeros(image.size(), image.type());
    filledGreen.setTo(cv::Scalar(0, 255, 0), maskGreen); // Green (BGR format)

    filledPink = cv::Mat::zeros(image.size(), image.type());
    filledPink.setTo(cv::Scalar(203, 192, 255), maskPink); // Pink (BGR format)

    // Combine all filled masks into the final image
    finalImage = filledBlue | filledOrange | filledRed | filledGreen | filledPink;

    return finalImage;
}


