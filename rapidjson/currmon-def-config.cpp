#include <iostream>
#include <array>

#include "include/rapidjson/document.h"
#include "include/rapidjson/prettywriter.h"
#include "include/rapidjson/istreamwrapper.h"
#include <fstream>

enum CurrmonTriggerMode { TriggerAmplitude, TriggerDio, TriggerNone };
enum CurrmonInputMode {InputCurrent, InputVoltage, InputNone };
#define CURRMON_CHANNELS_PER_BOARD 16
#define CURRMON_DEFAULT_MIN_LOG_DURATION 5
#define CURRMON_DEFAULT_MAX_LOG_DURATION 30
#define CURRMON_ID_LENGTH 16

#define CURRMON_DEFAULT_OVERSAMPLE 0
#define CURRMON_DEFAULT_SAMPLE_RATE 200.0
#define CURRMON_DEFAULT_LOGGING_RATE 100.0

struct CurrmonBoardConfig
{
  char currmon_id[CURRMON_ID_LENGTH + 1];
  std::array<CurrmonTriggerMode, CURRMON_CHANNELS_PER_BOARD> TriggerMode;
  std::array<int, CURRMON_CHANNELS_PER_BOARD> MinLogDuration;
  std::array<int, CURRMON_CHANNELS_PER_BOARD> MaxLogDuration;
  std::array<int, CURRMON_CHANNELS_PER_BOARD> Oversample;
  double SampleRate;
  CurrmonInputMode InputMode;
  std::array<bool, CURRMON_CHANNELS_PER_BOARD> Enabled;
  double LoggingRate;
  std::array<double, CURRMON_CHANNELS_PER_BOARD> AmplitudeStartThreshold;
  std::array<double, CURRMON_CHANNELS_PER_BOARD> AmplitudeStopThreshold;

  CurrmonBoardConfig () : InputMode(InputCurrent),
                  LoggingRate(CURRMON_DEFAULT_LOGGING_RATE),
                  SampleRate(CURRMON_DEFAULT_SAMPLE_RATE)
  {
      memset(currmon_id, 0, sizeof(currmon_id));
      TriggerMode.fill(TriggerNone);
      MinLogDuration.fill(CURRMON_DEFAULT_MIN_LOG_DURATION);
      MaxLogDuration.fill(CURRMON_DEFAULT_MAX_LOG_DURATION);
      Oversample.fill(CURRMON_DEFAULT_OVERSAMPLE);
      Enabled.fill(true);
      AmplitudeStartThreshold.fill(1);
      AmplitudeStopThreshold.fill(0.5);
  }

  template <typename Writer>
  void Serialize(Writer& writer) const {
    writer.StartObject();
    writer.Key("currmon_id");
    writer.String(currmon_id);
    writer.Key("TriggerMode");
    writer.StartArray();
    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      writer.Int(TriggerMode[i]);
    }
    writer.EndArray();
    writer.Key("MinLogDuration");
    writer.StartArray();
    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      writer.Int(MinLogDuration[i]);
    }
    writer.EndArray();
    writer.Key("MaxLogDuration");
    writer.StartArray();
    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      writer.Int(MaxLogDuration[i]);
    }
    writer.EndArray();
    writer.Key("Oversample");
    writer.StartArray();
    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      writer.Int(Oversample[i]);
    }
    writer.EndArray();
    writer.Key("SampleRate");
    writer.Double(SampleRate);
    writer.Key("InputMode");
    writer.Int(InputMode);
    writer.Key("Enabled");
    writer.StartArray();
    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      writer.Bool(Enabled[i]);
    }
    writer.EndArray();
    writer.Key("LoggingRate");
    writer.Double(LoggingRate);
    writer.Key("AmplitudeStartThreshold");
    writer.StartArray();
    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      writer.Double(AmplitudeStartThreshold[i]);
    }
    writer.EndArray();
    writer.Key("AmplitudeStopThreshold");
        writer.StartArray();
    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      writer.Double(AmplitudeStopThreshold[i]);
    }
    writer.EndArray();
    writer.EndObject();
  }

  void DeSerialize (rapidjson::Document *document)
  {
    rapidjson::Value& Val = document[0]["currmon_id"];

    strncpy(currmon_id, Val.GetString(), CURRMON_ID_LENGTH);

    Val = (*document)["TriggerMode"];

    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      const rapidjson::Value& mode = Val[i];
      TriggerMode[i] = CurrmonTriggerMode(mode.GetInt());
    }

    Val = (*document)["MinLogDuration"];

    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      const rapidjson::Value& mode = Val[i];
      MinLogDuration[i] = mode.GetInt();
    }

    Val = (*document)["MaxLogDuration"];

    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      const rapidjson::Value& mode = Val[i];
      MaxLogDuration[i] = mode.GetInt();
    }

    Val = (*document)["Oversample"];

    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      const rapidjson::Value& mode = Val[i];
      Oversample[i] = mode.GetInt();
    }

    Val = (*document)["SampleRate"];
    SampleRate = Val.GetDouble();

    Val = (*document)["InputMode"];
    InputMode = CurrmonInputMode(Val.GetInt());

    Val = (*document)["Enabled"];

    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      const rapidjson::Value& mode = Val[i];
      Enabled[i] = mode.GetBool();
    }

    Val = (*document)["LoggingRate"];
    LoggingRate = Val.GetDouble();


    Val = (*document)["AmplitudeStartThreshold"];

    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      const rapidjson::Value& mode = Val[i];
      AmplitudeStartThreshold[i] = mode.GetDouble();
    }

    Val = (*document)["AmplitudeStopThreshold"];

    for (int i = 0 ; i < CURRMON_CHANNELS_PER_BOARD ; i++)
    {
      const rapidjson::Value& mode = Val[i];
      AmplitudeStopThreshold[i] = mode.GetDouble();
    }
  }
};

int main (int argc, char **argv)
{
  rapidjson::StringBuffer s;
  rapidjson::Writer<rapidjson::StringBuffer> writer(s);

  CurrmonBoardConfig DefConfig;

  writer.StartObject();
  writer.Key("Configurations");
  writer.StartArray();
  DefConfig.Serialize(writer);
  DefConfig.Serialize(writer);
  writer.EndArray();
  writer.EndObject();

  std::cout << s.GetString() << std::endl;

  {
    CurrmonBoardConfig ReadConfigs[2];
    std::ifstream instream("config.json");
    rapidjson::IStreamWrapper wrapper(instream);

    rapidjson::Document document;
    document.ParseStream(wrapper);

    const rapidjson::Value& val = document["Configurations"];

    assert(val.IsArray());

    rapidjson::Document element;
    element.CopyFrom(val[0], element.GetAllocator());

    // rapidjson::StringBuffer sb;
    // rapidjson::Writer<rapidjson::StringBuffer> element_writer(sb);
    // element.Accept(element_writer);

    //std::cout<< sb.GetString() << std::endl;

    ReadConfigs[0].DeSerialize(&element);

    s.Clear();
    writer.Reset(s);
    ReadConfigs[0].Serialize(writer);

    std::cout << std::endl << std::endl;

    std::cout << s.GetString() << std::endl;


  }
  return 0;

}