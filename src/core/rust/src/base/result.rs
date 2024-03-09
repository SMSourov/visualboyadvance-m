#[repr(C)]
pub enum Result<T, E> {
    Ok(T),
    Err(E),
}
