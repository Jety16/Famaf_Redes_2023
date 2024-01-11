Informe : LABORATORIO 1 Aplicación Cliente

Integrante:
- Bazan Tomas

---

## Pregunta:
#### ¿Por qué funciona los dominios con nombres con emoticones o caracteres Asiaticos (no ASCII) ?

Eso se debe a que cuando nosotros hacemos la solicitud de un socket a otro socket debemos codificar la peticiones. Esto es lo que se denomina como encoding. Python implementa formato UTF-8 de manera predeterminada, este formato es lo que permite que funcionen dominios con estas características. Si por ejemplo fuese ASCII el predeterminado esto no sería posible.
```python
connection.send(HTTP_REQUEST % url.encode())
```
En esta linea de la funcion send_request se es donde efectivamente sucede el encoding.

---