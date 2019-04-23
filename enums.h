#ifndef ENUMS_H
#define ENUMS_H

enum class MessageId : unsigned char
{
    ChangeCduModeId = 1,
    ChangeRcModeId = 2,
    RegistrationOnId = 3,
    RegistrationOffId = 4,
    AnswerToQuestionId = 5,
    BoltJointOnId = 6,
    BoltJointOffId = 7,
    OperatorTrackCoordinateId = 8,
    OperatorActionId = 9,
    DefectMarkId = 10,
    RailroadSwitchMarkId = 11,
    TrackMapId = 12,
    NextTrackCoordinateId = 13,
    JumpTrackCoordinateId = 14,
    ManipulatorStateId = 15,
    PingId = 16,
    RailTypeId = 17
};

enum class OperatorAction : unsigned char
{
    PutFlagAction = 1,
    PutSwitchLocker = 2,
};

enum class CduMode : unsigned char
{
    ServiceMarksMode = 1,
    TrackMarksMode = 2,
    CalibrationMode = 3,
    BScanMode = 4,
    HandMode = 5,
    EvaluationMode = 6,
    PauseMode = 7,
};

enum class RcMode : unsigned char
{
    MainMode = 1,
    AnswerMode = 2,
};

enum class RailroadSide : unsigned char
{
    LeftSide = 1,
    RightSide = 2,
};

enum class Direction : unsigned char
{
    IncreaseDirection = 1,
    DecreaseDirection = 2,
};

struct MessageHeader
{
    unsigned char Id;  // eMESSAGEID
    unsigned char Reserved1;
    unsigned short Size;  // размер блока данных, который следует за заголовком
};

#endif  // ENUMS_H
