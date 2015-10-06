package com.jamesdanylik.hestia;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.Editable;
import android.view.View;
import android.widget.EditText;
import android.widget.Button;



public class HestiaLoginActivity extends Activity
{
	public static final String PREFS_NAME = "HestiaPrefs";

	EditText editTextAddress, editTextPort;
	private String address, port;

	Button btnConnect, btnClear;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.login);

        editTextAddress = (EditText)findViewById(R.id.editTextAddress);
        editTextPort = (EditText)findViewById(R.id.editTextPort);

        btnClear = (Button)findViewById(R.id.btnClear);
        btnConnect = (Button)findViewById(R.id.btnConnect);

        btnClear.setOnClickListener(new View.OnClickListener () {
          public void onClick(View v) {
            editTextAddress.setText("");
            editTextPort.setText("");
          }
        });

        btnConnect.setOnClickListener(new View.OnClickListener () {
          public void onClick(View v) {
            address = editTextAddress.getText().toString();
            port = editTextPort.getText().toString();

            Intent i = new Intent();
            i.putExtra("address", address);
            i.putExtra("port", port);
            setResult(RESULT_OK, i);
            finish();
          }

        });

        SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
        address = settings.getString("address",  "");
        port = settings.getString("port", "");

        editTextAddress.setText(address);
        editTextPort.setText(port);
    }

    @Override
    protected void onStop(){
       super.onStop();

        address = editTextAddress.getText().toString();
        port = editTextPort.getText().toString();

      // We need an Editor object to make preference changes.
      // All objects are from android.context.Context
      SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
      SharedPreferences.Editor editor = settings.edit();
      editor.putString("address", address);
      editor.putString("port", port);

      // Commit the edits!
      editor.commit();
    }

}
