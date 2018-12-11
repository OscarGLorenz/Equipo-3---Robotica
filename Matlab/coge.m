%%Funcion coge necesita donde esta ahora y a donde lleva la ficha
function coge(ahora, hasta)
global ardu;

offset = 30;
p = 3;
T = 10;

wp1 = ahora;
%%Punto a ir para recoger ficha
[x2,y2,z2] = directa(33.000,46,-100);
z2 = -80;
wp2 = [x2,y2,z2]; wp2(3) = wp2(3) + offset;
wp3 = [x2,y2,z2];
%%Punto para colocar ficha
wp4 = hasta; wp4(3) = wp4(3) + offset;
wp5 = hasta; 

move(wp1,wp2,T,p);      pause(T);
fprintf(ardu,"J4 M2 255");
move(wp2,wp3,T/4,p);      pause(T/4);
move(wp3,wp2,T/4,p);      pause(T/4);

move(wp2,wp4,T,p);      pause(T);

move(wp4,wp5,T/4,p);      pause(T/4);
fprintf(ardu,"J4 M2 0");
move(wp5,wp4,T/4,p);      pause(T/4);

move(wp4,wp1,T,p);      pause(T);
move(wp1,wp2,T,p);      pause(T);

end


function move(x0,xf,T,p)
global ardu;

% Muestrear en el espacio de la tarea
x1 = linspace(x0(1),xf(1),p);
x2 = linspace(x0(2),xf(2),p);
x3 = linspace(x0(3),xf(3),p);

% Muestrar el tiempo
t = linspace(0,T,p);

% Cinemática inversa de los puntos
[q1,q2,q3] = inversa(x1,x2,x3);

q2 = rad2deg(q2);
q3 = rad2deg(q3);

% Obtener los polinomios para coordenada articular con pendiente nula al
% principio y al final
pp1 = spline(t,[0 q1 0]);
pp2 = spline(t,[0 q2 0]);
pp3 = spline(t,[0 q3 0]);

% Generar mensaje en formato  d1 d2 d3 c1 c2 c3 b1 b2 b3 a1 a2 a3 t1 t2 t3
str1 = sprintf(' %3.3f',[pp1.coefs(:);t(2:end)']);
str2 = sprintf(' %3.3f',[pp2.coefs(:);t(2:end)']);
str3 = sprintf(' %3.3f',[pp3.coefs(:);t(2:end)']);

command = sprintf('J5 %d%s_%d%s_%d%s_\n', p-1, str1, p-1, str2, p-1, str3);

fprintf(ardu,command);
end
