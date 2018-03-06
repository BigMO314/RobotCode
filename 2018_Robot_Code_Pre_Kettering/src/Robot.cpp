#include "MOLib.h"
#include "Dashboard.h"
#include "Configuration/Goal.h"
#include "Configuration/Robot.h"
#include "ControlPeriod/Autonomous.h"
#include "ControlPeriod/TeleOperated.h"
#include "RobotMechanism/Arm.h"
#include "RobotMechanism/Drivetrain.h"

class Lockdown : public frc::SampleRobot {
public:
	Lockdown() {
		//delete everything in SmartDashboard, clears clutter
		nt::DeleteAllEntries(nt::GetDefaultInstance());

		Dashboard.Drivetrain.Inverted.Set(false);

		Dashboard.Drivetrain.Distance.P.Set(0.00015);	// 0.09
		Dashboard.Drivetrain.Distance.I.Set(0.0);		// 0.00
		Dashboard.Drivetrain.Distance.D.Set(0.00025);	//11.00

		Dashboard.Drivetrain.Angle.P.Set(0.0215); //0.024
		Dashboard.Drivetrain.Angle.I.Set(0.0);
		Dashboard.Drivetrain.Angle.D.Set(0.024); //0.0201

		Dashboard.Arm.Angle.P.Set(1.0);
		Dashboard.Arm.Angle.I.Set(0.0);
		Dashboard.Arm.Angle.D.Set(0.0);

		rbt_Drivetrain->ResetAngle();
		rbt_Drivetrain->ResetDistance();

		//Drivetrain
		mtr_L_Drive_1->SetInverted(true);
		mtr_L_Drive_2->SetInverted(true);
		mtr_L_Drive_3->SetInverted(true);

		mtr_L_Drive_2->Follow(*mtr_L_Drive_1);
		mtr_L_Drive_3->Follow(*mtr_L_Drive_1);
		mtr_L_Drive_1->ConfigSelectedFeedbackSensor(FeedbackDevice::CTRE_MagEncoder_Absolute, 0, 0);
		mtr_R_Drive_1->SetInverted(false);
		mtr_R_Drive_2->SetInverted(false);
		mtr_R_Drive_3->SetInverted(false);
		mtr_R_Drive_2->Follow(*mtr_R_Drive_1);
		mtr_R_Drive_3->Follow(*mtr_R_Drive_1);
		mtr_R_Drive_1->ConfigSelectedFeedbackSensor(FeedbackDevice::CTRE_MagEncoder_Absolute, 0, 0);

		rbt_Drivetrain->ConfigScale(0.6336, 0.66, -0.66, -0.66);

		//Arm
		mtr_Angle_1->SetInverted(false);
		mtr_Angle_2->SetInverted(false);
		mtr_Shooter1->SetInverted(false);
		mtr_Shooter2->SetInverted(false);
		mtr_Angle_2->Follow(*mtr_Angle_1);
		mtr_Angle_1->ConfigSelectedFeedbackSensor(FeedbackDevice::CTRE_MagEncoder_Absolute, 0, 0);

		mtr_Angle_1->ConfigForwardSoftLimitThreshold(95.0 / Configuration::Robot::Arm::DegreesPerCount, 0);
		mtr_Angle_1->ConfigReverseSoftLimitThreshold(-95.0 / Configuration::Robot::Arm::DegreesPerCount, 0);

		mtr_Angle_1->ConfigForwardSoftLimitEnable(true, 0);
		mtr_Angle_1->ConfigReverseSoftLimitEnable(true, 0);

		rbt_Arm->ConfigPeakOutput(0.5, -0.5);
		rbt_Arm->ResetAngle();

		//Intake
		mtr_L_Intake->SetInverted(true);
		mtr_R_Intake->SetInverted(false);

		std::cout << ">>Calibrating Gyro..." << std::endl;
		gyr_DriveAngle->Calibrate();
		std::cout << ">>Gyro Calibrated" << std::endl;

		SmartDashboard::PutData("AutonChooser", &prd_Autonomous->chs_Auton);
		SmartDashboard::PutData("PositionChooser", &prd_Autonomous->chs_Position);

	}

	void RobotInit() {

		rbt_Drivetrain->ConfigWheelDiameter(Configuration::Robot::Drivetrain::WheelDiameter);
	}

	void Autonomous() {
		prd_Autonomous->AutonInit();
		gyr_DriveAngle->Reset();

		while(IsAutonomous() && IsEnabled()){
			prd_Autonomous->Update();
			rbt_Drivetrain->Update();
			rbt_Arm->Update();
			frc::Wait(0.005);
		}
	}

	void OperatorControl() override {
		rbt_Drivetrain->DisableDistancePID();
		while (IsOperatorControl() && IsEnabled()) {
			//Update inputs
			prd_TeleOperated->Update();
			rbt_Drivetrain->Update();
			rbt_Arm->Update();

			frc::Wait(0.005);
		}
	}

	void Test() override {}

private:
	//Talons and Victors for Drivetrain																												_
	CTRLib::TalonSRX				*mtr_L_Drive_1		= new CTRLib::TalonSRX(0);
	CTRLib::VictorSPX				*mtr_L_Drive_2		= new CTRLib::VictorSPX(1);
	CTRLib::VictorSPX				*mtr_L_Drive_3		= new CTRLib::VictorSPX(2);
	CTRLib::TalonSRX				*mtr_R_Drive_1		= new CTRLib::TalonSRX(3);
	CTRLib::VictorSPX				*mtr_R_Drive_2		= new CTRLib::VictorSPX(4);
	CTRLib::VictorSPX				*mtr_R_Drive_3		= new CTRLib::VictorSPX(5);
			VictorSP				*mtr_Shooter1		= new		  VictorSP(8);
			VictorSP				*mtr_Shooter2		= new		  VictorSP(10);

	WPILib::ADXRS450_Gyro			*gyr_DriveAngle		= new ADXRS450_Gyro(SPI::kOnboardCS0);
	MOLib::Sensors::MagEncoder		*enc_DriveDistance	= new MOLib::Sensors::MagEncoder(mtr_L_Drive_1);

	//Create Drivetrain object																														_
	RobotMechanism::Drivetrain		*rbt_Drivetrain		= new RobotMechanism::Drivetrain(mtr_L_Drive_1,mtr_R_Drive_1,gyr_DriveAngle);

	//Talon and Victor for Arm																													_
	CTRLib::TalonSRX				*mtr_Angle_1		= new CTRLib::TalonSRX(6);
	CTRLib::VictorSPX				*mtr_Angle_2		= new CTRLib::VictorSPX(7);

	WPILib::Solenoid				*sol_Winch			= new WPILib::Solenoid(0);


	//Victors for Intake																															_
	WPILib::VictorSP				*mtr_L_Intake		= new WPILib::VictorSP(0);
	WPILib::VictorSP				*mtr_R_Intake		= new WPILib::VictorSP(1);
	//Intake sensors																																_
	WPILib::DigitalInput			*lim_Shooter		= new WPILib::DigitalInput(9);

	//Create Arm object
	RobotMechanism::Arm				*rbt_Arm			= new RobotMechanism::Arm(mtr_Angle_1, mtr_Angle_2, mtr_L_Intake, mtr_R_Intake, mtr_Shooter1, mtr_Shooter2, sol_Winch, lim_Shooter);

	//Controllers for TeleOperated																													_
	MOLib::XboxController			*ctl_Driver			= new MOLib::XboxController(0);
	MOLib::XboxController			*ctl_Operator		= new MOLib::XboxController(1);

	//Create Control Periods																														_
	ControlPeriod::TeleOperated		*prd_TeleOperated	= new ControlPeriod::TeleOperated(rbt_Drivetrain,rbt_Arm,ctl_Driver,ctl_Operator);
	ControlPeriod::Autonomous		*prd_Autonomous		= new ControlPeriod::Autonomous(rbt_Drivetrain,rbt_Arm);

};

START_ROBOT_CLASS(Lockdown);
