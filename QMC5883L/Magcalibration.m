close all; 
clearvars;

filename = ['LoggedData.csv']; % put yours raw mag data 
M = readmatrix(filename);

X_mag = M(:,1); 
Y_mag = M(:,2); 
Z_mag = M(:,3); 


figure(1)
plot3(X_mag(:),Y_mag(:),Z_mag(:),"LineStyle","none","Marker","X","MarkerSize",8, "MarkerFaceColor","k")
hold on
grid(gca,"on")

D = [X_mag(:),Y_mag(:),Z_mag(:)];
[A,b,expmfs] = magcal(D); % Calibration coefficients
expmfs; % Display the expected magnetic field strength in uT

C = (D-b)*A; % Calibrated data

plot3(C(:,1),C(:,2),C(:,3),"LineStyle","none","Marker","o", ...
      "MarkerSize",8,"MarkerFaceColor","r")
hold on


xlabel("uT")
ylabel("uT")
zlabel("uT")
legend("Nieskalibrowane pomiary","Skalibrowane pomiary","Location","southoutside")
title("Nieskalibrowany vs Skalibrowany")
