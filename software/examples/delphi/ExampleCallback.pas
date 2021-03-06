program ExampleCallback;

{$ifdef MSWINDOWS}{$apptype CONSOLE}{$endif}
{$ifdef FPC}{$mode OBJFPC}{$H+}{$endif}

uses
  SysUtils, IPConnection, BrickletEnergyMonitor;

type
  TExample = class
  private
    ipcon: TIPConnection;
    em: TBrickletEnergyMonitor;
  public
    procedure EnergyDataCB(sender: TBrickletEnergyMonitor; const voltage: longint;
                           const current: longint; const energy: longint;
                           const realPower: longint; const apparentPower: longint;
                           const reactivePower: longint; const powerFactor: word;
                           const frequency: word);
    procedure Execute;
  end;

const
  HOST = 'localhost';
  PORT = 4223;
  UID = 'XYZ'; { Change XYZ to the UID of your Energy Monitor Bricklet }

var
  e: TExample;

{ Callback procedure for energy data callback }
procedure TExample.EnergyDataCB(sender: TBrickletEnergyMonitor; const voltage: longint;
                                const current: longint; const energy: longint;
                                const realPower: longint; const apparentPower: longint;
                                const reactivePower: longint; const powerFactor: word;
                                const frequency: word);
begin
  WriteLn(Format('Voltage: %f V', [voltage/100.0]));
  WriteLn(Format('Current: %f A', [current/100.0]));
  WriteLn(Format('Energy: %f Wh', [energy/100.0]));
  WriteLn(Format('Real Power: %f h', [realPower/100.0]));
  WriteLn(Format('Apparent Power: %f VA', [apparentPower/100.0]));
  WriteLn(Format('Reactive Power: %f var', [reactivePower/100.0]));
  WriteLn(Format('Power Factor: %f', [powerFactor/1000.0]));
  WriteLn(Format('Frequency: %f Hz', [frequency/100.0]));
  WriteLn('');
end;

procedure TExample.Execute;
begin
  { Create IP connection }
  ipcon := TIPConnection.Create;

  { Create device object }
  em := TBrickletEnergyMonitor.Create(UID, ipcon);

  { Connect to brickd }
  ipcon.Connect(HOST, PORT);
  { Don't use device before ipcon is connected }

  { Register energy data callback to procedure EnergyDataCB }
  em.OnEnergyData := {$ifdef FPC}@{$endif}EnergyDataCB;

  { Set period for energy data callback to 1s (1000ms) }
  em.SetEnergyDataCallbackConfiguration(1000, false);

  WriteLn('Press key to exit');
  ReadLn;
  ipcon.Destroy; { Calls ipcon.Disconnect internally }
end;

begin
  e := TExample.Create;
  e.Execute;
  e.Destroy;
end.
