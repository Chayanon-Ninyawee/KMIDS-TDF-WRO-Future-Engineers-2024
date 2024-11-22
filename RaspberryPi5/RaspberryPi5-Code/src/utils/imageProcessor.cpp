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
    // Remove the top 40% of the image
    int cropHeight = static_cast<int>(image.rows * CROP_PERCENT);
    cv::Rect cropRegion(0, cropHeight, image.cols, image.rows - cropHeight);
    cv::Mat croppedImage = image(cropRegion);

    cv::Mat hsvImage;
    cv::cvtColor(croppedImage, hsvImage, cv::COLOR_BGR2HSV);

    // Masks for blue and orange
    cv::Mat maskBlue, maskOrange;
    cv::inRange(hsvImage, lowerBlueLine, upperBlueLine, maskBlue);
    cv::inRange(hsvImage, lowerOrangeLine, upperOrangeLine, maskOrange);

    std::vector<std::vector<cv::Point>> contoursBlue, contoursOrange;
    cv::findContours(maskBlue, contoursBlue, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(maskOrange, contoursOrange, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Mat filteredMaskBlue = cv::Mat::zeros(maskBlue.size(), maskBlue.type());
    cv::Mat filteredMaskOrange = cv::Mat::zeros(maskOrange.size(), maskOrange.type());

    for (const auto &contour : contoursBlue) {
        if (cv::contourArea(contour) > minBlueLineArea) {
            cv::drawContours(filteredMaskBlue, std::vector<std::vector<cv::Point>>{contour}, -1, 255, cv::FILLED);
        }
    }

    for (const auto &contour : contoursOrange) {
        if (cv::contourArea(contour) > minOrangeLineArea) {
            cv::drawContours(filteredMaskOrange, std::vector<std::vector<cv::Point>>{contour}, -1, 255, cv::FILLED);
        }
    }

    std::vector<cv::Point> blueCoordinates = getCoordinates(filteredMaskBlue);
    std::vector<cv::Point> orangeCoordinates = getCoordinates(filteredMaskOrange);

    int blueLineY = getAverageY(blueCoordinates);
    int blueLineSize = blueCoordinates.size();
    int orangeLineY = getAverageY(orangeCoordinates);
    int orangeLineSize = orangeCoordinates.size();

    // Masks for red and green
    cv::Mat maskRed1, maskRed2, maskRed, maskGreen;
    cv::inRange(hsvImage, lowerRed1Light, upperRed1Light, maskRed1);
    cv::inRange(hsvImage, lowerRed2Light, upperRed2Light, maskRed2);
    maskRed = maskRed1 | maskRed2;
    cv::inRange(hsvImage, lowerGreenLight, upperGreenLight, maskGreen);

    std::vector<std::vector<cv::Point>> contoursRed, contoursGreen;
    cv::findContours(maskRed, contoursRed, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::findContours(maskGreen, contoursGreen, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<Block> redBlocks, greenBlocks;
    for (const auto &contour : contoursRed) {
        if (cv::contourArea(contour) > minRedLineArea) {
            auto [centroid, area, lowestPoint] = getCentroidAndArea(contour);
            redBlocks.push_back({centroid.x, centroid.y + cropHeight, lowestPoint.y + cropHeight, static_cast<int>(area), cv::Scalar(0, 0, 255)});
        }
    }

    for (const auto &contour : contoursGreen) {
        if (cv::contourArea(contour) > minGreenLineArea) {
            auto [centroid, area, lowestPoint] = getCentroidAndArea(contour);
            greenBlocks.push_back({centroid.x, centroid.y + cropHeight, lowestPoint.y + cropHeight, static_cast<int>(area), cv::Scalar(0, 255, 0)});
        }
    }

    // Mask for pink
    cv::Mat maskPink;
    cv::inRange(hsvImage, lowerPinkLight, upperPinkLight, maskPink);

    std::vector<cv::Point> pinkCoordinates = getCoordinates(maskPink);
    int pinkX = getAverageX(pinkCoordinates);
    int pinkY = getAverageY(pinkCoordinates);
    int pinkSize = pinkCoordinates.size();

    // Adjust pink coordinates for the cropped region
    if (pinkY >= 0) pinkY += cropHeight;

    // Construct result
    ImageProcessingResult result;
    result.blueLineY = blueLineY >= 0 ? blueLineY + cropHeight : blueLineY;
    result.blueLineSize = blueLineSize;
    result.orangeLineY = orangeLineY >= 0 ? orangeLineY + cropHeight : orangeLineY;
    result.orangeLineSize = orangeLineSize;
    result.blocks = redBlocks;
    result.blocks.insert(result.blocks.end(), greenBlocks.begin(), greenBlocks.end());
    result.pinkX = pinkX;
    result.pinkY = pinkY;
    result.pinkSize = pinkSize;

    return result;
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


float pixelToAngleInLidar(int pixelX, int imageWidth, float fov, float lidarCameraDistance) {
    const float DEG_TO_RAD = CV_PI / 180.0f;
    const float RAD_TO_DEG = 180.0f / CV_PI;

    float centerX = imageWidth / 2.0f;               // Center of the image
    float anglePerPixel = fov / imageWidth;         // Angular resolution per pixel
    float originalAngle = (pixelX - centerX) * anglePerPixel; // Original angle in degrees

    // Convert to radians for calculation
    float theta = originalAngle * DEG_TO_RAD;

    // Assume a normalized focal length (you can adjust if needed)
    float focalLength = 1.0f;

    // Adjust angle based on the virtual camera position
    float tanTheta = std::tan(theta);
    float tanPhi = tanTheta * focalLength / (focalLength - lidarCameraDistance * tanTheta);

    float phi = std::atan(tanPhi); // Adjusted angle in radians

    // Convert back to degrees and wrap if negative
    float adjustedAngle = phi * RAD_TO_DEG;
    if (adjustedAngle < 0) {
        adjustedAngle += 360.0f;
    }

    return adjustedAngle;
}


cv::Mat drawImageProcessingResult(const ImageProcessingResult &result, cv::Mat &image) {
    // Create a copy to draw on (optional)
    cv::Mat outputImage = image.clone();

    // Add the crop line
    int cropY = static_cast<int>(image.rows * CROP_PERCENT);
    cv::line(outputImage, cv::Point(0, cropY), cv::Point(image.cols, cropY), cv::Scalar(0, 255, 255), 2);
    cv::putText(outputImage, "Crop Line", cv::Point(10, cropY - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);

    // Draw blue line information
    if (result.blueLineY >= 0) {
        cv::line(outputImage, cv::Point(0, result.blueLineY), cv::Point(image.cols, result.blueLineY), cv::Scalar(255, 0, 0), 2);
        cv::putText(outputImage, "Blue Line", cv::Point(10, result.blueLineY - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
    }

    // Draw orange line information
    if (result.orangeLineY >= 0) {
        cv::line(outputImage, cv::Point(0, result.orangeLineY), cv::Point(image.cols, result.orangeLineY), cv::Scalar(0, 165, 255), 2);
        cv::putText(outputImage, "Orange Line", cv::Point(10, result.orangeLineY - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 165, 255), 1);
    }

    // Draw blocks (red and green)
    for (const auto &block : result.blocks) {
        cv::circle(outputImage, cv::Point(block.x, block.y), 5, block.color, -1);
        cv::putText(outputImage, 
                    "Size: " + std::to_string(block.size), 
                    cv::Point(block.x + 10, block.y - 10), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, block.color, 1);
        cv::putText(outputImage, 
                    "Lowest Y: " + std::to_string(block.lowestY), 
                    cv::Point(block.x + 10, block.y + 10), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, block.color, 1);
    }

    // Draw pink region information
    if (result.pinkSize > 0) {
        cv::circle(outputImage, cv::Point(result.pinkX, result.pinkY), 10, cv::Scalar(203, 192, 255), -1);
        cv::putText(outputImage, 
                    "Pink Size: " + std::to_string(result.pinkSize), 
                    cv::Point(result.pinkX + 15, result.pinkY), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(203, 192, 255), 1);
    }

    return outputImage;
}


