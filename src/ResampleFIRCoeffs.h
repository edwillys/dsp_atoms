float FIR_RESAMPLE_FAC2[41] = {
    -7.158971096141E-19F, -1.051458762959E-03F,  1.854299878147E-18F,  2.508966717869E-03F, -3.494145324274E-18F, -4.894834477454E-03F,  5.606451107282E-18F,  8.556558750570E-03F, 
    -8.091013394948E-18F, -1.398997288197E-02F,  1.078113213278E-17F,  2.202312089503E-02F, -1.345967778667E-17F, -3.434018045664E-02F,  1.588460076576E-17F,  5.528828501701E-02F, 
    -1.782020670786E-17F, -1.009301990271E-01F,  1.906929740929E-17F,  3.167003393173E-01F,  5.002587437630E-01F,  3.167003393173E-01F,  1.906929740929E-17F, -1.009301990271E-01F, 
    -1.782020670786E-17F,  5.528828501701E-02F,  1.588460076576E-17F, -3.434018045664E-02F, -1.345967778667E-17F,  2.202312089503E-02F,  1.078113213278E-17F, -1.398997288197E-02F, 
    -8.091013394948E-18F,  8.556558750570E-03F,  5.606451107282E-18F, -4.894834477454E-03F, -3.494145324274E-18F,  2.508966717869E-03F,  1.854299878147E-18F, -1.051458762959E-03F, 
    -7.158971096141E-19F, 
};

float FIR_RESAMPLE_FAC3[61] = {
    -4.773070465845E-19F, -5.075758090243E-04F, -7.171615143307E-04F,  1.236309520196E-18F,  1.275830902159E-03F,  1.636325730942E-03F, -2.329636735609E-18F, -2.551690209657E-03F, 
    -3.121165791526E-03F,  3.737965495220E-18F,  4.526422824711E-03F,  5.382746923715E-03F, -5.394486902496E-18F, -7.467288523912E-03F, -8.729113265872E-03F,  7.188058902382E-18F, 
     1.180763635784E-02F,  1.369067002088E-02F, -8.973913720743E-18F, -1.839848794043E-02F, -2.138582989573E-02F,  1.059067296956E-17F,  2.934103831649E-02F,  3.484134003520E-02F, 
    -1.188119135761E-17F, -5.182809010148E-02F, -6.626323610544E-02F,  1.271399183525E-17F,  1.365522146225E-01F,  2.751477360725E-01F,  3.335354030132E-01F,  2.751477360725E-01F, 
     1.365522146225E-01F,  1.271399183525E-17F, -6.626323610544E-02F, -5.182809010148E-02F, -1.188119135761E-17F,  3.484134003520E-02F,  2.934103831649E-02F,  1.059067296956E-17F, 
    -2.138582989573E-02F, -1.839848794043E-02F, -8.973913720743E-18F,  1.369067002088E-02F,  1.180763635784E-02F,  7.188058902382E-18F, -8.729113265872E-03F, -7.467288523912E-03F, 
    -5.394486902496E-18F,  5.382746923715E-03F,  4.526422824711E-03F,  3.737965495220E-18F, -3.121165791526E-03F, -2.551690209657E-03F, -2.329636735609E-18F,  1.636325730942E-03F, 
     1.275830902159E-03F,  1.236309520196E-18F, -7.171615143307E-04F, -5.075758090243E-04F, -4.773070465845E-19F, 
};

float FIR_RESAMPLE_FAC4[81] = {
    -3.579911546086E-19F, -2.826493873727E-04F, -5.257919547148E-04F, -4.754169785883E-04F,  9.272602642876E-19F,  7.316285045817E-04F,  1.254632719792E-03F,  1.063079806045E-03F, 
    -1.747280491266E-18F, -1.483012922108E-03F, -2.447708509862E-03F, -2.006531460211E-03F,  2.803559321018E-18F,  2.651272574440E-03F,  4.278788808733E-03F,  3.438489744440E-03F, 
    -4.045988116590E-18F, -4.394791088998E-03F, -6.995819043368E-03F, -5.554971285164E-03F,  5.391207544957E-18F,  6.966729182750E-03F,  1.101287081838E-02F,  8.698813617229E-03F, 
    -6.730639604168E-18F, -1.085577160120E-02F, -1.717213355005E-02F, -1.360637322068E-02F,  7.943245850321E-18F,  1.724308542907E-02F,  2.764743193984E-02F,  2.232083678246E-02F, 
    -8.911163799475E-18F, -3.003358840942E-02F, -5.047110468149E-02F, -4.349443688989E-02F,  9.535783596445E-18F,  7.413570582867E-02F,  1.583690196276E-01F,  2.249081432819E-01F, 
     2.501591444016E-01F,  2.249081432819E-01F,  1.583690196276E-01F,  7.413570582867E-02F,  9.535783596445E-18F, -4.349443688989E-02F, -5.047110468149E-02F, -3.003358840942E-02F, 
    -8.911163799475E-18F,  2.232083678246E-02F,  2.764743193984E-02F,  1.724308542907E-02F,  7.943245850321E-18F, -1.360637322068E-02F, -1.717213355005E-02F, -1.085577160120E-02F, 
    -6.730639604168E-18F,  8.698813617229E-03F,  1.101287081838E-02F,  6.966729182750E-03F,  5.391207544957E-18F, -5.554971285164E-03F, -6.995819043368E-03F, -4.394791088998E-03F, 
    -4.045988116590E-18F,  3.438489744440E-03F,  4.278788808733E-03F,  2.651272574440E-03F,  2.803559321018E-18F, -2.006531460211E-03F, -2.447708509862E-03F, -1.483012922108E-03F, 
    -1.747280491266E-18F,  1.063079806045E-03F,  1.254632719792E-03F,  7.316285045817E-04F,  9.272602642876E-19F, -4.754169785883E-04F, -5.257919547148E-04F, -2.826493873727E-04F, 
    -3.579911546086E-19F, 
};
