package com.jamesdanylik.hestia;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;


import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.CheckBox;
import android.widget.ArrayAdapter;

import org.apache.commons.net.telnet.TelnetClient;

import java.io.InputStream;
import java.io.PrintStream;

public class HestiaMainActivity extends Activity
{
    String address;
    int port;

    TelnetClient telnet = new TelnetClient();
    Boolean connected = false;
    InputStream tcIn;
    PrintStream tcOut;

    TextView textViewLightState, textViewMotorState;

    Button  btnRefresh,
            btnToggleLight,
            btnToggleShow,
            btnToggleAll,
            btnToggleOsc,
            btnToggleMotor,
            btnToggleDir,
            btnResetMotor,
            btnActMotor;

    Spinner spinLight;
    String spinLightSelected;
    CheckBox checkboxDevice;


    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Intent i = new Intent(this, HestiaLoginActivity.class);
        startActivityForResult(i, 1);
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode == 1) {
            if (resultCode == RESULT_OK) {
                address = data.getStringExtra("address");
                port = Integer.parseInt(data.getStringExtra("port"));

                try {
                    telnet.connect(address, port);
                }
                catch (Exception e) {
                    restartActivity();
                }

                connected = true;
                tcIn = telnet.getInputStream();
                tcOut = new PrintStream(telnet.getOutputStream());

                setContentView(R.layout.main);

                spinLight = (Spinner)findViewById(R.id.spinLight);
                ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this, R.array.relay_lights_array, android.R.layout.simple_spinner_item);
                // Specify the layout to use when the list of choices appears
                adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                // Apply the adapter to the spinner
                spinLight.setAdapter(adapter);
                spinLight.setOnItemSelectedListener(new OnItemSelectedListener() {
                    @Override
                    public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                        String selected = spinLight.getSelectedItem().toString();
                        spinLightSelected = selected.substring(selected.length() - 1);
                    }

                    @Override
                    public void onNothingSelected(AdapterView<?> arg0) {
                        spinLightSelected = "1";
                    }
                });

                checkboxDevice = (CheckBox)findViewById(R.id.checkboxDevice);

                textViewLightState = (TextView)findViewById(R.id.textViewLightState);
                textViewMotorState = (TextView)findViewById(R.id.textViewMotorState);

                btnRefresh = (Button)findViewById(R.id.btnRefresh);
                btnToggleLight = (Button)findViewById(R.id.btnToggleLight);
                btnToggleShow = (Button)findViewById(R.id.btnToggleShow);
                btnToggleAll = (Button)findViewById(R.id.btnToggleAll);
                btnToggleOsc = (Button)findViewById(R.id.btnToggleOsc);
                btnToggleMotor = (Button)findViewById(R.id.btnToggleMotor);
                btnResetMotor = (Button)findViewById(R.id.btnResetMotor);
                btnToggleDir = (Button)findViewById(R.id.btnToggleDir);
                btnActMotor = (Button)findViewById(R.id.btnActMotor);

                btnRefresh.setOnClickListener(new View.OnClickListener () {
                    public void onClick(View v) {
                        sendCommand("ZZ");
                        String response = readUntil("\n");
                        response = response.substring(0, response.length() - 1);
                        textViewLightState.setText(response);
                        textViewMotorState.setText(response);
                    }
                });               

                btnToggleLight.setOnClickListener(new View.OnClickListener () {
                    public void onClick(View v) {
                        if(checkboxDevice.isChecked())     
                            sendCommand("Z"+spinLightSelected);
                        else
                            sendCommand("L"+spinLightSelected);
                    }
                });

                btnToggleShow.setOnClickListener(new View.OnClickListener () {
                    public void onClick(View v) {
                        sendCommand("ZS");
                    }
                });

                btnToggleAll.setOnClickListener(new View.OnClickListener () {
                    public void onClick(View v) {
                        sendCommand("ZT");
                    }
                });

                btnToggleOsc.setOnClickListener(new View.OnClickListener () {
                    public void onClick(View v) {
                        sendCommand("ZO");
                    }
                });

                btnToggleMotor.setOnClickListener(new View.OnClickListener () {
                    public void onClick(View v) {
                        sendCommand("ZM");
                    }
                });

                btnResetMotor.setOnClickListener(new View.OnClickListener () {
                    public void onClick(View v) {
                        sendCommand("ZR");
                    }
                });

                btnToggleDir.setOnClickListener(new View.OnClickListener () {
                    public void onClick(View v) {
                        sendCommand("ZD");
                    }
                });

                btnActMotor.setOnClickListener(new View.OnClickListener () {
                    public void onClick(View v) {
                        sendCommand("ZA");
                    }
                });

            }
            if (resultCode == RESULT_CANCELED) {
                restartActivity();
            }
        }
    }

    protected void restartActivity() {
        Intent intent = getIntent();
        finish();
        startActivity(intent); 
    }

    protected void sendCommand(String command) {
        tcOut.println(command);
        tcOut.flush();
        readUntil("#"+command+"\n");
    }

    protected void updateStatus() {
        sendCommand("ZZ");
        String response = readUntil("\n");
        response = response.substring(0, response.length() - 1);
        textViewLightState.setText(response);
        textViewMotorState.setText(response);
    }

    @Override
    protected void onStop() {
        super.onStop();  // Always call the superclass method first

        if (connected) {
            try {
                telnet.disconnect();
            }
            catch (Exception e) {
                e.printStackTrace();
            }
            connected = false;
            restartActivity();
        }
    }

    @Override
    public void onPause() {
        super.onPause();  // Always call the superclass method first

        if (connected) {
            try {
                telnet.disconnect();
            }
            catch (Exception e) {
                e.printStackTrace();
            }
            connected = false;
            restartActivity();
        }
    }

    @Override
    public void onResume() {
        super.onResume();  // Always call the superclass method first

    }

    @Override
    protected void onRestart() {
        super.onRestart();  // Always call the superclass method first
        if (!connected) {
            restartActivity();
        }  
    }

    public String readUntil(String pattern) {
        try {
            char lastChar = pattern.charAt(pattern.length() - 1);
            StringBuffer sb = new StringBuffer();
            boolean found = false;
            char ch = (char) tcIn.read();
            while (true) {
                sb.append(ch);
                if (ch == lastChar) {
                    if (sb.toString().endsWith(pattern)) {
                        return sb.toString();
                    }
                }
            ch = (char) tcIn.read();
            }
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }
}
