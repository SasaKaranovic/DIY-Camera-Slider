/*
CameraSlider - Web
Description: This file contains all the functions we use for receiving and responding to HTTP requests
*/

#include "version.h"

// Helper function that allows us to replace template variable in .html file
// with a value from our running code.
// ie. Any instance of %RAIL_LENGTH% will be replaced with the actual
// value of `SliderConfig.Config.rail_length` in our firmware
String template_const_processor(const String& var)
{
    if(var == "RAIL_LENGTH") {
        return String(SliderConfig.Config.rail_length);
    }
    else if (var == "FW_VERSION") {
        return String(String(VERSION_MAJOR) + "." + String(VERSION_MINOR) + "." + String(VERSION_PATCH));
    }
    else if (var == "HOMING_SPEED_SLIDER") {
        return String(SliderConfig.Config.homing_speed_slide);
    }
    else if (var == "RAIL_LENGTH") {
        return String(SliderConfig.Config.rail_length);
    }
    else if (var == "MIN_STEP_SLIDER") {
        return String(SliderConfig.Config.min_slider_step);
    }
    else if (var == "HOMING_SPEED_ROTATION") {
        return String(SliderConfig.Config.homing_speed_pan);
    }
    else if (var == "SLIDER_STEPS_PER_MM") {
        return String(SliderConfig.Config.slide_steps_per_mm);
    }
    else if (var == "ROTATION_STEPS_PER_MM") {
        return String(SliderConfig.Config.pan_steps_per_degree);
    }
    else if (var == "CHECK_BOX_HOMING_INVERTED") {
        if(SliderConfig.Config.homing_direction == 1) {
            return String("");
        }
        else {
            return String("checked");
        }
    }
    else if (var == "CHECK_BOX_SLIDING_INVERTED") {
        if(SliderConfig.Config.slider_direction == 1) {
            return String("");
        }
        else {
            return String("checked");
        }
    }
    else if (var == "CHECK_BOX_ROTATION_INVERTED") {
        if(SliderConfig.Config.rotate_direction == 1) {
            return String("");
        }
        else {
            return String("checked");
        }
    }

    Serial.print("Unknown template variable: ");
    Serial.println(var);
    return String();
}


void setupWebServer(void)
{
    server.onNotFound([](AsyncWebServerRequest *request) {
        Serial.println("404:");
        Serial.println(request->url());
        request->send(404);
    });

    // send a file when /index is requested
    server.on("/index.html", HTTP_ANY, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html", false, template_const_processor);
    });

    server.on("/settings.html", HTTP_ANY, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/settings.html", "text/html", false, template_const_processor);
    });

    server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html", false, template_const_processor);
    });

    server.serveStatic("/js/", LittleFS, "/js/").setCacheControl("max-age=2592000");;
    server.serveStatic("/css/", LittleFS, "/css/").setCacheControl("max-age=2592000");;
    server.serveStatic("/webfonts/", LittleFS, "/webfonts/").setCacheControl("max-age=2592000");;
    server.serveStatic("/favicon.png", LittleFS, "/favicon.png").setCacheControl("max-age=2592000");;
    server.serveStatic("/favicon.ico", LittleFS, "/favicon.ico").setCacheControl("max-age=2592000");;

    // Homing request - Sliding
    server.on("/api/home-slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Sliding rail HOME request received...");

        if(CameraSlider_SetState(SLIDER_HOMING)) {
            request->send(200, "text/plain", "OK");
        }
        else {
            request->send(500, "text/plain", "INVALID STATE");
        }
    });

    // Homing request - Rotation
    server.on("/api/home-rotation", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Rotation HOME request received...");
        CameraSlider_StoreAsRotationHome();
        request->send(200, "text/plain", "OK");
    });

    // Motors Off request
    server.on("/api/motors-turn-off", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Turning motors OFF");

        CameraSlider_EnableMotors(false);
        request->send(200, "text/plain", "OK");
    });

    // Motors On request
    server.on("/api/motors-turn-on", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Turning motors ON");

        CameraSlider_EnableMotors(true);
        request->send(200, "text/plain", "OK");
    });

    // Get status
    server.on("/api/camera-slider-status", HTTP_GET, [] (AsyncWebServerRequest *request) {
        char buff[300] = {0};

        if(CameraSlider_FormatJSON_CameraSliderStatus(buff, 200))
        {
            request->send(200, "text/plain", buff);
        }
        else
        {
            request->send(500, "text/plain", "CameraSlider_FormatJSON_CameraSliderStatus failed");
        }
    });

    // Get camera slider config
    server.on("/api/camera-slider-config", HTTP_GET, [] (AsyncWebServerRequest *request) {
        char buff[300] = {0};

        if(CameraSlider_FormatJSON_CameraConfig(buff, 300))
        {
            request->send(200, "text/plain", buff);
        }
        else
        {
            request->send(500, "text/plain", "CameraSlider_FormatJSON_CameraSliderStatus failed");
        }
    });

    // Set START position
    server.on("/api/position-save-start", HTTP_GET, [] (AsyncWebServerRequest *request) {
        CameraSlider_StoreAsStartPosition();
        request->send(200, "text/plain", "OK");
    });


    // Set START position
    server.on("/api/position-save-end", HTTP_GET, [] (AsyncWebServerRequest *request) {
        CameraSlider_StoreAsEndPosition();
        request->send(200, "text/plain", "OK");
    });

    // Move to start Position
    server.on("/api/position-goto-start", HTTP_GET, [] (AsyncWebServerRequest *request) {
        WebAPI_MoveToPosition(MOVE_TO_STORED_POSITION_START, request);
    });

    // Move to end position
    server.on("/api/position-goto-end", HTTP_GET, [] (AsyncWebServerRequest *request) {
        WebAPI_MoveToPosition(MOVE_TO_STORED_POSITION_END, request);
    });


    // Transition camera from start position to end position within request time frame
    // Our has two modes
    // 1. We move camera to start/stop position and save it. Then ask camera to slide within X second
    // 2. We manually specify start/stop position in mm and ask camera to slide within X seconds
    server.on("/api/move-start-to-stop", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Moving camera from start to end position");

        if(CameraSlider_getMotorState() == false) {
            Serial.println("Motors are OFF");
            request->send(409, "text/plain", "Motors are OFF");
            return;
        }
        else if(CameraSlider_getHomingState() == false) {
            Serial.println("Homing not complete!");
            request->send(409, "text/plain", "Homing not complete! Please home your camera slider first.");
            return;
        }

        // At a minimum we need seconds parameter
        if ( request->hasParam("seconds") ) {
            uint32_t u32Seconds = 0.0;
            u32Seconds = request->getParam("seconds")->value().toInt();

            if ( request->hasParam("startPos") && request->hasParam("endPos") && request->hasParam("rotateBy") ) {
                Serial.println("Start-Stop position explicitly specified");
                float fStartPos = 0.0;
                float fEndPos = 0.0;
                float fRotateBy = 0.0;

                fStartPos = request->getParam("startPos")->value().toFloat();
                fEndPos = request->getParam("endPos")->value().toFloat();
                fRotateBy = request->getParam("rotateBy")->value().toFloat();

                CameraSlider_SetStartPosition(fStartPos, 0.0);
                CameraSlider_SetEndPosition(fEndPos, fRotateBy);
                CameraSlider_SetDuration(u32Seconds);

                Serial.println("Starting motion");
                CameraSlider_StartMotion();

                request->send(200, "text/plain", "OK");
                return;
            }
            else {
                Serial.println("Start-Stop position pre-saved.");
                CameraSlider_SetDuration(u32Seconds);
                Serial.print("u32Seconds: ");
                Serial.println(u32Seconds);

                Serial.println("Starting motion");
                CameraSlider_StartMotion();

                request->send(200, "text/plain", "OK");
                return;
            }

        }
    });

    // Move to location
    server.on("/api/move-to-position", HTTP_GET, [] (AsyncWebServerRequest *request) {
        WebAPI_MoveToPosition(MOVE_RELATIVE, request);
    });

    // Configure camera - Set rail length
    server.on("/api/set-rail-length", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Updating rail length");
        uint32_t length = 0;

        if ( request->hasParam("value") ) {
            // SLIDER
            length = request->getParam("value")->value().toInt();
            CameraSlider_UpdateRailLength(length);
            request->send(200, "text/plain", "OK");
        }
        else {
            Serial.print("Invalid request!");
            request->send(400, "text/plain", "Malformed request");
        }
    });

    // Configure camera - Set homing direction
    server.on("/api/set-homing-direction", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Updating homing direction");
        if (WebAPI_UpdateMotorConfig(HOMING_DIRECTION, request)) {
            request->send(200, "text/plain", "OK");
            return;
        }
        else {
            request->send(400, "text/plain", "Bad Request");
            return;
        }
    });

    // Configure camera - Set sliding direction
    server.on("/api/set-slide-direction", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Updating slide direction");
        if (WebAPI_UpdateMotorConfig(SLIDING_DIRECTION, request)) {
            request->send(200, "text/plain", "OK");
            return;
        }
        else {
            request->send(400, "text/plain", "Bad Request");
            return;
        }
    });

    // Configure camera - Set pan direction
    server.on("/api/set-pan-direction", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Updating pan direction");
        if (WebAPI_UpdateMotorConfig(PAN_DIRECTION, request)) {
            request->send(200, "text/plain", "OK");
            return;
        }
        else {
            request->send(400, "text/plain", "Bad Request");
            return;
        }
    });

    // Configure camera - Slider steps per mm
    server.on("/api/set-steps-per-mm", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Updating steps per mm");
        if (WebAPI_UpdateMotorConfig(SLIDER_STEPS_PER_MM, request)) {
            request->send(200, "text/plain", "OK");
            return;
        }
        else {
            request->send(400, "text/plain", "Bad Request");
            return;
        }
    });

    // Configure camera - Rotation steps per mm
    server.on("/api/set-steps-per-deg", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Updating steps per degree");
        if (WebAPI_UpdateMotorConfig(ROTATION_STEPS_PER_DEG, request)) {
            request->send(200, "text/plain", "OK");
            return;
        }
        else {
            request->send(400, "text/plain", "Bad Request");
            return;
        }
    });

    // Configure camera - Homing speed slider
    server.on("/api/set-homing-speed-slide", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Updating homing speed for slider");
        if (WebAPI_UpdateMotorConfig(HOMING_SPEED_SLIDE, request)) {
            request->send(200, "text/plain", "OK");
            return;
        }
        else {
            request->send(400, "text/plain", "Bad Request");
            return;
        }

    });

    // Configure camera - Homing speed pan
    server.on("/api/set-homing-speed-pan", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Updating homing speed for pan");
        if(WebAPI_UpdateMotorConfig(HOMING_SPEED_PAN, request)) {
            request->send(200, "text/plain", "OK");
            return;
        }
        else {
            request->send(400, "text/plain", "Bad Request");
            return;
        }
    });

    // Configure camera - Homing speed pan
    server.on("/api/set-slider-min-step", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Updating minimum slider step");
        if(WebAPI_UpdateMotorConfig(MIN_SLIDER_STEP, request)) {
            request->send(200, "text/plain", "OK");
            return;
        }
        else {
            request->send(400, "text/plain", "Bad Request");
            return;
        }
    });

    // Configure camera - Reset settings to their default values
    server.on("/api/settings-reset", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("Resetting settings to default values");
        SliderConfig.ResetToDefault();
    });

}

// Helper function to move camera slider into requested position
// arguments
//      - move_type -> enum indicating relative or absolute positioning
//      - request   -> HTTP request pointer

void WebAPI_MoveToPosition(CameraSliderMovement_t move_type, AsyncWebServerRequest *request)
{
    if(CameraSlider_getMotorState() == false) {
        request->send(409, "text/plain", "Motors are OFF");
        return;
    }

    Serial.println("Movement request received...");
    float fSlidePos     = 0.0;
    float fSlideSpeed   = SliderConfig.Config.default_slider_speed;
    float fSlideAccel   = SliderConfig.Config.default_slider_accel;
    float fRotPos       = 0.0;
    float fRotSpeed     = SliderConfig.Config.default_rotate_speed;
    float fRotAccel     = SliderConfig.Config.default_rotate_accel;

    if ( request->hasParam("xPos") ) {
        fSlidePos = request->getParam("xPos")->value().toFloat();
    }

    if ( request->hasParam("xSpeed") ) {
        fSlideSpeed = request->getParam("xSpeed")->value().toFloat();
    }

    if ( request->hasParam("xAccel") ) {
        fSlideAccel = request->getParam("xAccel")->value().toFloat();
    }

    if ( request->hasParam("rPos") ) {
        fRotPos = request->getParam("rPos")->value().toFloat();
    }

    if ( request->hasParam("xPos") ) {
        fRotSpeed = request->getParam("rSpeed")->value().toFloat();
    }

    if ( request->hasParam("xPos") ) {
        fRotAccel = request->getParam("rAccel")->value().toFloat();
    }

    // Debug printout
    Serial.print("xPosition: ");
    Serial.println(fSlidePos);

    Serial.print("xSpeed: ");
    Serial.println(fSlideSpeed);

    Serial.print("xAccel: ");
    Serial.println(fSlideAccel);

    Serial.print("rPosition: ");
    Serial.println(fRotPos);

    Serial.print("rSpeed: ");
    Serial.println(fRotSpeed);

    Serial.print("rAccel: ");
    Serial.println(fRotAccel);

    if ( move_type == MOVE_RELATIVE) {
        CameraSlider_MoveToPositionRelative(fSlidePos, fSlideSpeed,  fSlideAccel, fRotPos, fRotSpeed, fRotAccel);
        request->send(200, "text/plain", "OK");
        return;
    }
    else if (move_type == MOVE_TO_STORED_POSITION_START) {
        CameraSlider_MoveToStart(fSlideSpeed,  fSlideAccel, fRotSpeed, fRotAccel);
        request->send(200, "text/plain", "OK");
    }
    else if (move_type == MOVE_TO_STORED_POSITION_END) {
        CameraSlider_MoveToEnd(fSlideSpeed,  fSlideAccel, fRotSpeed, fRotAccel);
        request->send(200, "text/plain", "OK");
    }
    else {
        Serial.print("Invalid request!");
        request->send(400, "text/plain", "Malformed request");
    }
}


// Helper function to retrieve integer value from HTTP request
// arguments
//      - request   -> HTTP request pointer
//      - argName   -> name of the http argument
//      - pInt      -> pointer to where we want value to be stored
// returns
//      - true      -> value has been found in HTTP request and stored in pInt
//      - false     -> HTTP request does NOT contain valid value. pInt was not modified
bool WebAPI_GetIntValueFromRequest(AsyncWebServerRequest *pRequest, const char *argName, int32_t *pInt)
{
    if (pInt == NULL) {
        Serial.println("Invalid pInt pointer!");
        return false;
    }

    if (pRequest == NULL) {
        Serial.println("Invalid pRequest pointer!");
        return false;
    }

    if (argName == NULL) {
        Serial.println("Invalid argName pointer!");
        return false;
    }

    if ( pRequest->hasParam( (const __FlashStringHelper *) argName) ) {
        *pInt = pRequest->getParam((const __FlashStringHelper *) argName)->value().toInt();
        return true;
    }

    return false;
}

// Helper function to update SliderConfig from a HTTP request
// arguments
//      - parameter -> enum indicating which parameter we want changed
//      - request   -> HTTP request pointer
// returns
//      - true      -> value has been succesfully updated
//      - false     -> failed to update value
bool WebAPI_UpdateMotorConfig(CameraSliderConfig_t parameter, AsyncWebServerRequest *pRequest)
{
    int32_t value = 0;

    if( !WebAPI_GetIntValueFromRequest(pRequest, "value", &value) ) {
        Serial.println("Value not found!");
        return false;
    }

    Serial.print("New value: ");
    Serial.println(value, DEC);

    // Below code could be reduced by moving .Write() and return statements
    // outside of switch() statement. However this might be confusing to young players,
    // so we are laving the code in a more "verbose" state since code size and execution speed
    // is not a huge concern for this project
    switch(parameter) {
        case HOMING_DIRECTION:
            SliderConfig.Config.homing_direction = value;
            SliderConfig.Write();
            return true;
        break;

        case SLIDING_DIRECTION:
            SliderConfig.Config.slider_direction = value;
            SliderConfig.Write();
            return true;
        break;

        case PAN_DIRECTION:
            SliderConfig.Config.rotate_direction = value;
            SliderConfig.Write();
            return true;
        break;

        case SLIDER_STEPS_PER_MM:
            SliderConfig.Config.slide_steps_per_mm = value;
            SliderConfig.Write();
            return true;
        break;

        case ROTATION_STEPS_PER_DEG:
            SliderConfig.Config.pan_steps_per_degree = value;
            SliderConfig.Write();
            return true;
        break;

        case HOMING_SPEED_SLIDE:
            SliderConfig.Config.homing_speed_slide = value;
            SliderConfig.Write();
            return true;
        break;

        case HOMING_SPEED_PAN:
            SliderConfig.Config.homing_speed_pan = value;
            SliderConfig.Write();
            return true;
        break;

        case MIN_SLIDER_STEP:
            SliderConfig.Config.min_slider_step = value;
            SliderConfig.Write();
            return true;
        break;

        default:
            return false;
        break;
    }

    return false;
}
