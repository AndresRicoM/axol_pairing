#ifndef TIMESERVER_H
#define TIMESERVER_H

// Time Server Variables
namespace timeserver
{
    String formattedDate;
    String dayStamp;
    String timeStamp;
    const char *ntpServer = "pool.ntp.org";
    long gmtOffset_sec = 0;
    const int daylightOffset_sec = 0;

    void get_time()
    { // Functiuons queries server to get current time. Activates time screen.

        // Init and get the time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time");
            return;
        }

        // int num_month = month(timeinfo);
        // Serial.println(num_month);

        char buffer[80]; // Buffer to hold the formatted string. Adjust the size as needed.
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        formattedDate = String(buffer);
        // Serial.println(formattedDate);

        // Extract date
        int splitT = formattedDate.indexOf(" ");
        dayStamp = formattedDate.substring(0, splitT);
        Serial.print("DATE: ");
        // Serial.println(dayStamp);

        // Extract time
        timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 3);
        Serial.print("HOUR: ");
        // Serial.println(timeStamp);
    }
} // namespace timeserver

#endif // TIMESERVER_H