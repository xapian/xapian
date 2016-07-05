package org.xapian.example.android;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.util.List;

public class MainActivity extends AppCompatActivity {
    private static final String DB_FOLDER = "testdb2";
    private static final String TAG = "MainActivity";
    String DB_PATH;
    EditText et;
    Button index,search;
    TextView tv_result;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        try {
            SmokeTest.main(null);
        } catch (Exception e) {
            e.printStackTrace();
        }
        DB_PATH=getFilesDir().getAbsolutePath()+"/"+DB_FOLDER;
        Log.d(TAG,DB_PATH);
        et= (EditText) findViewById(R.id.editText);
        index= (Button) findViewById(R.id.b_index);
        search= (Button) findViewById(R.id.b_search);
        tv_result= (TextView) findViewById(R.id.tv_results);
        index.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String sentence=et.getText().toString();
                String words[]=sentence.split(" ");
                try {
                    SimpleIndex.index(DB_PATH,words);
                } catch (Exception e) {
                    e.printStackTrace();
                }

            }
        });
        search.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String sentence=et.getText().toString();
                String words[]=sentence.split(" ");
                try {
                    tv_result.setText("");
                    List<String> results= SimpleSearch.search(DB_PATH,words);
                    if (results==null){
                        tv_result.setText("No docs found");
                    }else {
                        for (String s : results) {
                            tv_result.append(s+"\n");
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });

    }
}
