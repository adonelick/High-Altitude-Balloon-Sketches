// Written by Andrew Donelick
// 13 December 2015
// adonelick@hmc.edu


#include "DataFile.h"

DataFile::DataFile(uint8_t chipSelect, uint8_t cardDetect)
    : filenameIndex_(0),
      numEntries_(0),
      currentEntry_(0),
      arduinoType_(UNO),
      chipSelect_(chipSelect),
      cardDetect_(cardDetect)
{
    strcpy(filename_, "DATA000.CSV");
}

DataFile::DataFile(int arduinoType, uint8_t chipSelect, uint8_t cardDetect)
    : filenameIndex_(0),
      numEntries_(0),
      currentEntry_(0),
      arduinoType_(arduinoType),
      chipSelect_(chipSelect),
      cardDetect_(cardDetect)
{
    strcpy(filename_, "DATA000.CSV");
}

void DataFile::begin()
{
    // Start the SD card
    // These pins need to be set to output for writing to the
    // SD card, even if we do not use them. Which pin is used
    // as the SS pin depends on the type of Arduino

    pinMode(cardDetect_, INPUT);
    if (!digitalRead(cardDetect_)) {
        return;
    }

    SD.begin(chipSelect_);

    filenameIndex_ = 0;
    while (SD.exists(filename_) && (filenameIndex_ < 999))
    {
        ++filenameIndex_;
        filename_[4] = filenameIndex_/100 + '0';
        filename_[5] = (filenameIndex_%100)/10 + '0';
        filename_[6] = (filenameIndex_%100)%10 + '0';
    }
}

void DataFile::begin(char const* filename)
{
    pinMode(cardDetect_, INPUT);
    if (!digitalRead(cardDetect_)) {
        return;
    }

    SD.begin(chipSelect_);
    strcpy(filename_, filename);
}

void DataFile::open()
{
    if (!digitalRead(cardDetect_)) {
        return;
    }

    dataFile_ = SD.open(filename_, FILE_WRITE);
}

void DataFile::close()
{
    if (!digitalRead(cardDetect_)) {
        return;
    }

    dataFile_.close();
}

void DataFile::addEntry(char const* entryName)
{
    if (numEntries_ < NUM_ENTRIES) {
        entries_[numEntries_] = entryName;
        ++numEntries_;
    }
}

void DataFile::writeFileHeader()
{
    open();
    if (dataFile_)
    {
        for (int i = 0; i < numEntries_; ++i)
        {
            dataFile_.print("Time,");
            dataFile_.print(entries_[i]);

            if (i != numEntries_ - 1)
            {
                dataFile_.print(',');
            }
        }
        dataFile_.flush();

        writeNewLine();
        close();
    }
}

void DataFile::writeEntry(int value)
{
    if (!dataFile_)
        return;
    writeEntryTime();
    dataFile_.print(value);
    dataFile_.flush();
    writeEntryEnd();
}

void DataFile::writeEntry(unsigned int value)
{
    if (!dataFile_)
        return;
    writeEntryTime();
    dataFile_.print(value);
    dataFile_.flush();
    writeEntryEnd();
}

void DataFile::writeEntry(unsigned long value)
{
    if (!dataFile_)
        return;
    writeEntryTime();
    dataFile_.print(value);
    dataFile_.flush();
    writeEntryEnd();
}

void DataFile::writeEntry(float value)
{
    if (!dataFile_)
        return;
    writeEntryTime();
    dataFile_.print(value);
    dataFile_.flush();
    writeEntryEnd();
}

void DataFile::writeEntry(double value)
{
    if (!dataFile_)
        return;
    writeEntryTime();
    dataFile_.print(value);
    dataFile_.flush();
    writeEntryEnd();
}

void DataFile::writeEntry(bool value)
{
    if (!dataFile_)
        return;

    writeEntryTime();
    if (value) {
        dataFile_.print("True");
    } else {
        dataFile_.print("False");
    }
    dataFile_.flush();
    writeEntryEnd();
}

void DataFile::writeEntry(char const* value)
{
    if (!dataFile_)
        return;
    writeEntryTime();
    dataFile_.print(value);
    dataFile_.flush();
    writeEntryEnd();
}


void DataFile::writeEntryTime()
{
    if (!dataFile_)
        return;

    dataFile_.print(millis());
    dataFile_.print(',');
    dataFile_.flush();
}


void DataFile::writeEntryEnd()
{
    
    if (!dataFile_)
        return;

    ++currentEntry_;

    if (currentEntry_ != numEntries_) {
        dataFile_.print(',');
        dataFile_.flush();
    } else {
        currentEntry_ = 0;
        writeNewLine();
    }
}


// This function assumes that the file is open
void DataFile::writeNewLine()
{
    if (dataFile_)
    {
        dataFile_.println();
        dataFile_.flush();
    }
}

bool DataFile::checkStatus()
{
    // No card exists
    if (!digitalRead(cardDetect_)) {
        return false;
    }

    if (dataFile_) {
        return true;
    } else {
        return false;
    }
}

char* DataFile::getFilename()
{
    return filename_;
}

unsigned int DataFile::getFilenameIndex()
{
    return filenameIndex_;
}

bool DataFile::exists()
{
    return SD.exists(filename_);
}